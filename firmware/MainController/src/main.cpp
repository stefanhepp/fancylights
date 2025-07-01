/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Controller main loop implementation.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */

#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

#include <commands.h>

#include "config.h"
#include "CommandLine.h"
#include "LED.h"
#include "Settings.h"
#include "ProjectorController.h"
#include "MqttBroker.h"

Settings settings;
CommandLine cmdline;
LEDDriver LEDs(settings);
ProjectorController Projector(settings);

HardwareSerial keypadSerial(UART_KEYPAD);

static const int UART_BUFFER_SIZE = 16;
static uint8_t UARTBuffer[UART_BUFFER_SIZE];
static uint8_t UARTBufferLength = 0;

uint8_t CntStatusTimeout = 0;

MqttBroker mqttBroker(settings);

void sendStatus();

class StatusParser: public CommandParser
{
    public:
        StatusParser() {}

        static void printRGBMode(RGBMode mode) {
            switch (mode) {
                case RGB_ON:
                    Serial.print("on");
                    break;
                case RGB_CYCLE:
                    Serial.print("cycle");
                    break;
                case RGB_FIRE:
                    Serial.print("fire");
                    break;
                case RGB_DIMMED:
                    Serial.print("dimmed");
                    break;
            }
        }

        static void printProjectorCommand(ProjectorCommand cmd) {
            switch (cmd) {
                case PROJECTOR_OFF:
                    Serial.print("off");
                    break;
                case PROJECTOR_ON:
                    Serial.print("on");
                    break;
                case PROJECTOR_NORMAL:
                    Serial.print("normal");
                    break;
                case PROJECTOR_3D:
                    Serial.print("3D");
                    break;
                case PROJECTOR_VR:
                    Serial.print("VR");
                    break;
            }
        }

        static void printLiftCommand(LiftCommand cmd) {
            switch (cmd) {
                case LIFT_UP:
                    Serial.print("up");
                    break;
                case LIFT_DOWN:
                    Serial.print("down");
                    break;
                case LIFT_STOP:
                    Serial.print("stop");
                    break;
            }
        }

        static void printBool(bool value) {
            Serial.print(value ? "on" : "off");
        }

        virtual CmdExecStatus completeCommand(bool expectCommand) {
            Serial.printf("Enable Lamp: "); printBool(LEDs.isLampEnabled());
            Serial.printf(", LED Strip: "); printBool(LEDs.isLEDStripEnabled()); Serial.println();
            Serial.printf("Light Intensity: %hhu\n", LEDs.intensity());
            Serial.printf("Dimmed Intensity: %hhu\n", LEDs.dimmedIntensity());
            Serial.printf("RGB Strip Mode: "); printRGBMode(LEDs.rgbMode()); Serial.println();
            Serial.printf("Projector Mode: "); printProjectorCommand(Projector.mode()); Serial.println();
            Serial.printf("HSV: %hhu %hhu %hhu\n", LEDs.getHSV().hue, LEDs.getHSV().sat, LEDs.getHSV().val);
            Serial.printf("WiFi SSID: %s PW: %s\n", settings.getWiFiSSID(), settings.getWiFiPassword());
            Serial.printf("WiFi Hostname %s IP %s\n", WiFi.getHostname(), WiFi.localIP().toString());
            Serial.printf("WiFi Status (%d) %s\n", WiFi.status(), WiFi.status() == WL_CONNECTED ? "connected" : "not connected");
            Serial.printf("MQTT Server %s:%d\n", settings.getMQTTServer(), settings.getMQTTPort());
            Serial.printf("MQTT Client %s Topic %s\n", settings.getMQTTClientID(), settings.getMQTTTopic());
            Serial.printf("MQTT User %s Pass: %s\n", settings.getMQTTUsername(), settings.getMQTTPassword());
            Serial.printf("MQTT Status %s\n", mqttBroker.connected() ? "connected" : "not connected");

            Projector.requestStatus();

            return CmdExecStatus::CESOK;
        }
};

class WiFiParser: public CommandParser {
    private:
        enum WiFiCommand {
            WIC_NONE,
            WIC_AP,
            WIC_HOSTNAME,
            WIC_CONNECT
        };
        WiFiCommand mCmd;

        String mSSID;
        String mPassword;
        String mHostname;

    public:
        WiFiParser() {}

        virtual void printArguments() {
            Serial.print("ap <ssid> <password>|hostname <hostname>|connect");
        }

        virtual CmdParseStatus startCommand(const char* cmd) {
            mCmd = WIC_NONE;
            return CPSNextArgument;
        }

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) {
            if (argNo == 0) {
                if (strcmp(arg, "ap") == 0) {
                    mCmd = WIC_AP;
                    return CPSNextArgument;
                }
                if (strcmp(arg, "hostname") == 0) {
                    mCmd = WIC_HOSTNAME;
                    return CPSNextArgument;
                }
                if (strcmp(arg, "CONNECT") == 0) {
                    mCmd = WIC_CONNECT;
                    return CPSComplete;
                }
            }
            if (mCmd == WIC_AP && argNo == 1) {
                mSSID = arg;
                return CPSNextArgument;
            }
            if (mCmd == WIC_AP && argNo == 2) {
                mPassword = arg;
                return CPSComplete;
            }
            if (mCmd == WIC_HOSTNAME && argNo == 1) {
                mHostname = arg;
                return CPSComplete;
            }
            return CPSInvalidArgument;
        }

        virtual CmdExecStatus completeCommand(bool expectCommand) {
            if (mCmd == WIC_AP) {
                settings.setWiFiAccess(mSSID.c_str(), mPassword.c_str());
                WiFi.disconnect();
                WiFi.begin(mSSID.c_str(), mPassword.c_str());
                WiFi.reconnect();
                return CmdExecStatus::CESOK;
            }
            if (mCmd == WIC_HOSTNAME) {
                settings.setHostname(mHostname.c_str());
                WiFi.setHostname(mHostname.c_str());
                return CmdExecStatus::CESOK;
            }
            if (mCmd == WIC_CONNECT) {
                WiFi.disconnect();
                WiFi.reconnect();
                Serial.printf("[WiFi] Reconnect: %s status %d IP %s\n",
                              WiFi.isConnected() ? "connected" : "not connected",
                              WiFi.status(),
                              WiFi.localIP().toString().c_str());
                return CmdExecStatus::CESOK;
            }
            return CmdExecStatus::CESInvalidArgument;
        }
};

class MQTTParser: public CommandParser {
    private:
        enum MqttCommand {
            MC_NONE,
            MC_SERVER,
            MC_TOPIC,
            MC_CLIENT,
            MC_USER
        };

        MqttCommand mCmd;
        String mMqttHostname;
        int    mMqttPort;
        String mTopic;
        String mClientID;
        String mUser;
        String mPassword;

    public:
        MQTTParser() {}

        virtual void printArguments() {
            Serial.print("server <hostname> <port>|topic <topic>|client <clientID>|user <user> <pw>");
        }

        virtual CmdParseStatus startCommand(const char* cmd) {
            mCmd == MC_NONE;
            return CPSNextArgument;
        }

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) {
            if (argNo == 0) {
                if (strcmp(arg, "server") == 0) {
                    mCmd = MC_SERVER;
                    return CPSNextArgument;
                }
                if (strcmp(arg, "topic") == 0) {
                    mCmd = MC_TOPIC;
                    return CPSNextArgument;
                }
                if (strcmp(arg, "client") == 0) {
                    mCmd = MC_CLIENT;
                    return CPSNextArgument;
                }
                if (strcmp(arg, "user") == 0) {
                    mCmd = MC_USER;
                    return CPSNextArgument;
                }
            }
            if (mCmd == MC_SERVER && argNo == 1) {
                mMqttHostname = arg;
                return CPSNextArgument;
            }
            if (mCmd == MC_SERVER && argNo == 2) {
                if (parseInteger(arg, mMqttPort, 0, 65535)) {
                    return CPSComplete;
                } else {
                    return CPSInvalidArgument;
                }
            }
            if (mCmd == MC_TOPIC && argNo == 1) {
                mTopic = arg;
                return CPSComplete;
            }
            if (mCmd == MC_CLIENT && argNo == 1) {
                mClientID = arg;
                return CPSComplete;
            }
            if (mCmd == MC_USER && argNo == 1) {
                mUser = arg;
                return CPSNextArgument;
            }
            if (mCmd == MC_USER && argNo == 2) {
                mPassword = arg;
                return CPSComplete;
            }
            return CPSInvalidArgument;
        }

        virtual CmdExecStatus completeCommand(bool expectCommand) {
            // TODO reconnect MQTT connection
            if (mCmd == MC_SERVER) {
                settings.setMQTTServer(mMqttHostname.c_str(), mMqttPort, settings.getMQTTTopic().c_str());
                return CESOK;
            }
            if (mCmd == MC_TOPIC) {
                settings.setMQTTServer(settings.getMQTTServer().c_str(), settings.getMQTTPort(), mTopic.c_str());
                return CESOK;
            }
            if (mCmd == MC_CLIENT) {
                settings.setMQTTClient(mClientID.c_str(), settings.getMQTTUsername().c_str(), settings.getMQTTPassword().c_str());
                return CESOK;
            }
            if (mCmd == MC_USER) {
                settings.setMQTTClient(settings.getMQTTClientID().c_str(), mUser.c_str(), mPassword.c_str());
                return CESOK;
            }
            return CmdExecStatus::CESInvalidArgument;
        }
};

class LEDParser: public CommandParser {
    private:
        CommandOpcode mCommand;
        RGBMode mMode;
        bool    mLEDEnable;
        CHSV    mColor;

    public:
        LEDParser() {}

        virtual void printArguments() {
            Serial.print("on|off|cycle|color <h> <s> <v>");
        }

        virtual CmdParseStatus startCommand(const char* cmd) {
            mCommand = CMD_READ_STATUS;

            return CPSNextArgument;
        }

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) {
            if (argNo == 0) {
                if (strcmp(arg, "on") == 0) {
                    mCommand = CMD_RGB_MODE;
                    mLEDEnable = true;
                    mMode = RGB_ON;
                    return CPSComplete;
                }
                if (strcmp(arg, "off") == 0) {
                    mCommand = CMD_RGB_MODE;
                    mLEDEnable = false;
                    mMode = RGB_ON;
                    return CPSComplete;
                }
                if (strcmp(arg, "cycle") == 0) {
                    mCommand = CMD_RGB_MODE;
                    mLEDEnable = true;
                    mMode = RGB_CYCLE;
                    return CPSComplete;
                }
                if (strcmp(arg, "color") == 0) {
                    mCommand = CMD_HSV_COLOR;
                    return CPSNextArgument;
                }
                return CPSInvalidArgument;
            }
            if (mCommand == CMD_HSV_COLOR) {
                if (argNo > 0 and argNo < 4) {
                    int v;
                    if (parseInteger(arg, v, 0, 255)) {
                        mColor[argNo-1] = v;
                        if (argNo < 3) {
                            return CPSNextArgument;
                        } else {
                            return CPSComplete;
                        }
                    } else {
                        return CPSInvalidArgument;
                    }
                }
            }
            return CPSInvalidArgument;
        }

        virtual CmdExecStatus completeCommand(bool expectCommand) {
            if (mCommand == CMD_RGB_MODE) {
                LEDs.enableLEDStrip(mLEDEnable);
                LEDs.enableLamps(mLEDEnable);
                LEDs.setRGBMode(mMode);
                sendStatus();
                return CmdExecStatus::CESOK;
            }
            if (mCommand == CMD_HSV_COLOR) {
                LEDs.setHSV(mColor.h, mColor.s, mColor.v);
                sendStatus();
                return CmdExecStatus::CESOK;
            }
            return CmdExecStatus::CESInvalidArgument;
        }
};

void sendStatus()
{
    keypadSerial.write(CMD_HEADER | CMD_READ_STATUS);
    keypadSerial.write((LEDs.isLEDStripEnabled() << 1) | (LEDs.isLampEnabled()));
    keypadSerial.write(LEDs.intensity());
    keypadSerial.write(LEDs.dimmedIntensity());
    keypadSerial.write(Projector.mode());
    keypadSerial.write(LEDs.rgbMode());
    keypadSerial.write(LEDs.getHSV().hue);
    keypadSerial.write(LEDs.getHSV().sat);
    keypadSerial.write(LEDs.getHSV().val);
}

void onProjectorStatus(bool switchState, bool powerOn) {
    CntStatusTimeout = 0;

    Serial.print("[Projector] Status: Power "); StatusParser::printBool(powerOn);
    Serial.print(", Switch: "); StatusParser::printBool(switchState);
    Serial.println();
}

void processCommand(uint8_t data)
{
    if (UARTBufferLength == 0 && (data & CMD_HEADER) != CMD_HEADER) {
        // Wait for command byte, drop other data
        return;
    }

    if (UARTBufferLength < UART_BUFFER_SIZE) {
        UARTBuffer[UARTBufferLength++] = data;
    }

    switch (UARTBuffer[0] & ~CMD_HEADER) {
        case CMD_LIGHT_INTENSITY:
            if (UARTBufferLength >= 2) {
                LEDs.setIntensity(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Set light intensity: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_DIMMED_INTENSITY:
            if (UARTBufferLength >= 2) {
                LEDs.setDimmedIntensity(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Set dimmed intensity: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_HSV_COLOR:
            if (UARTBufferLength >= 4) {
                LEDs.setHSV(UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Set HSV: %hhu %hhu %hhu\n", UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
            }
            break;
        case CMD_LIGHT_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.enableLamps(UARTBuffer[1] & 0x01);
                LEDs.enableLEDStrip(UARTBuffer[1] & 0x02);
                UARTBufferLength = 0;
                Serial.print("[Kbd] Enable lamp: "); StatusParser::printBool(UARTBuffer[1] & 0x01);
                Serial.print(", LED strip: ");       StatusParser::printBool(UARTBuffer[1] & 0x02);
                Serial.println();
            }
            break;
        case CMD_RGB_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.setRGBMode((RGBMode) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.print("[Kbd] Set RGB mode: "); StatusParser::printRGBMode((RGBMode) UARTBuffer[1]);
                Serial.println();
            }
            break;
        case CMD_SCREEN:
            if (UARTBufferLength >= 2) {
                Projector.moveScreen((LiftCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.print("[Kbd] Move Screen: "); StatusParser::printLiftCommand((LiftCommand) UARTBuffer[1]);
                Serial.println();
            }
            break;
        case CMD_PROJECTOR_MODE:
            if (UARTBufferLength >= 2) {
                Projector.setMode((ProjectorCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Set projector mode: "); StatusParser::printProjectorCommand((ProjectorCommand) UARTBuffer[1]);
                Serial.println();
            }
            break;
        case CMD_PROJECTOR_LIFT:
            if (UARTBufferLength >= 2) {
                Projector.moveProjector((LiftCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.print("[Kbd] Lift projector: "); StatusParser::printLiftCommand((LiftCommand) UARTBuffer[1]);
                Serial.println();
            }
            break;
        case CMD_REQUEST_STATUS:
            if (UARTBufferLength >= 2) {
                Projector.requestStatus();
                CntStatusTimeout = 10;
                UARTBufferLength = 0;
            }
            break;
        default:
            // unknown command, drop
            UARTBufferLength = 0;
            break;
    }
}

void checkWiFiConnection()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("[WiFi] Not connected. Reconnecting ..\n");
        WiFi.disconnect();
        WiFi.reconnect();
    }
}

void checkMQTTConnection() 
{
    if (WiFi.status() == WL_CONNECTED && !mqttBroker.connected())
    {
        mqttBroker.connect();
    }
}

void setup() {
    settings.begin();

    cmdline.begin();
    cmdline.addCommand("status", new StatusParser());
    cmdline.addCommand("led", new LEDParser());
    cmdline.addCommand("wifi", new WiFiParser());
    cmdline.addCommand("mqtt", new MQTTParser());

    LEDs.begin();

    Projector.setStatusCallback(onProjectorStatus);
    Projector.begin();

    keypadSerial.begin(UART_SPEED_CONTROLLER, SERIAL_8N1, PIN_KP_RXD, PIN_KP_TXD);

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(settings.getHostname().c_str());
    WiFi.begin(settings.getWiFiSSID(), settings.getWiFiPassword());

    mqttBroker.setup();
}

void loop() {
    // Process keypad UART cmds
    while (keypadSerial.available()) {
        char data = keypadSerial.read();

        processCommand(data);
    }

    if (CntStatusTimeout > 0) {
        CntStatusTimeout--;
        if (CntStatusTimeout == 0) {
            sendStatus();
        }
    }

    cmdline.loop();
    LEDs.loop();
    Projector.loop();
    mqttBroker.loop();

    EVERY_N_SECONDS( 20 ) {
        checkWiFiConnection();
        checkMQTTConnection();
    }
}
