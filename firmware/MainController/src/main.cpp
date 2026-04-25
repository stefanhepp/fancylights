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
#include "KeypadDriver.h"
#include "MqttClient.h"

Settings settings;
CommandLine cmdline;
MqttClient mqttClient(settings);
LEDDriver LEDs(settings, mqttClient);
ProjectorController Projector(settings, mqttClient);
KeypadDriver Keypad(settings, LEDs, Projector);


class StatusParser: public CommandParser
{
    public:
        StatusParser() {}

        virtual CmdExecStatus completeCommand(bool expectCommand) {
            IPAddress mqttAddr;
            WiFi.hostByName(settings.getMQTTServer().c_str(), mqttAddr);

            Serial.printf("Enable Lamp: %s, LED Strip: %s\n", strBool(LEDs.isLampEnabled()), strBool(LEDs.isLEDStripEnabled()));
            Serial.printf("Light Intensity: %hhu\n", LEDs.intensity());
            Serial.printf("Dimmed Intensity: %hhu\n", LEDs.dimmedIntensity());
            Serial.printf("RGB Strip Mode: %s\n", strRGBMode(LEDs.rgbMode()));
            Serial.printf("Projector Mode: %s\n", strProjectorCommand(Projector.mode()));
            Serial.printf("HSV: %hhu %hhu %hhu\n", LEDs.getHSV().hue, LEDs.getHSV().sat, LEDs.getHSV().val);
            Serial.printf("WiFi SSID: %s PW: %s\n", settings.getWiFiSSID(), settings.getWiFiPassword());
            Serial.printf("WiFi Hostname %s IP %s DNS %s\n", WiFi.getHostname(), WiFi.localIP().toString(), WiFi.dnsIP().toString());
            Serial.printf("WiFi Status (%d) %s\n", WiFi.status(), WiFi.status() == WL_CONNECTED ? "connected" : "not connected");
            Serial.printf("MQTT Server %s:%d IP: %s\n", settings.getMQTTServer(), settings.getMQTTPort(), mqttAddr.toString());
            Serial.printf("MQTT Client %s Topic %s\n", settings.getMQTTClientID(), settings.getMQTTTopic());
            Serial.printf("MQTT User %s Pass: %s\n", settings.getMQTTUsername(), settings.getMQTTPassword());
            Serial.printf("MQTT Status %s\n", mqttClient.connected() ? "connected" : "not connected");

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
            Serial.print("ap <ssid> <password>|hostname <hostname>|reconnect");
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
                if (strcmp(arg, "reconnect") == 0) {
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
            MC_USER,
            MC_CONNECT
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
            Serial.print("server <hostname> <port>|topic <topic>|client <clientID>|user <user> <pw>|reconnect");
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
                if (strcmp(arg, "reconnect") == 0) {
                    mCmd = MC_CONNECT;
                    return CPSComplete;
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
            if (mCmd == MC_CONNECT) {
                mqttClient.connect();
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
            Serial.print("on|off|cycle|spin|scan|fire|water|rainbow|bpm|color <h> <s> <v>");
        }

        virtual CmdParseStatus startCommand(const char* cmd) {
            mCommand = CMD_READ_STATUS;

            return CPSNextArgument;
        }

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) {
            if (argNo == 0) {
                if (parseRGBMode(arg, mMode)) {
                    mCommand = CMD_RGB_MODE;
                    mLEDEnable = true;
                    return CPSComplete;
                }
                if (strcmp(arg, "off") == 0) {
                    mCommand = CMD_RGB_MODE;
                    mLEDEnable = false;
                    mMode = RGB_ON;
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
                if (mLEDEnable) {
                    LEDs.setRGBMode(mMode);
                }
                return CmdExecStatus::CESOK;
            }
            if (mCommand == CMD_HSV_COLOR) {
                LEDs.setHSV(mColor.h, mColor.s, mColor.v);
                return CmdExecStatus::CESOK;
            }
            return CmdExecStatus::CESInvalidArgument;
        }
};

class ScreenParser: public CommandParser {
    private:
        LiftCommand mCmd;

    public:
        ScreenParser() {}

        virtual void printArguments() {
            Serial.print("up|down|stop");
        }

        virtual CmdParseStatus startCommand(const char* cmd) {
            return CPSNextArgument;
        }

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) {
            if (parseLiftCommand(arg, mCmd)) {
                return CPSComplete;
            }
            return CPSInvalidArgument;
        }

        virtual CmdExecStatus completeCommand(bool expectCommand) {
            Projector.moveScreen(mCmd);
            return CmdExecStatus::CESOK;
        }
};

class ProjectorParser: public CommandParser {
    private:
        LiftCommand mLiftCmd;
        ProjectorCommand mProjectorCmd;
        bool mIsLiftCommand = false;
        bool mIsProjectorCommand = false;

    public:
        ProjectorParser() {}

        virtual void printArguments() {
            Serial.print("up|down|on|off|3D");
        }

        virtual CmdParseStatus startCommand(const char* cmd) {
            mIsLiftCommand = false;
            mIsProjectorCommand = false;
            return CPSNextArgument;
        }

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) {
            if (parseLiftCommand(arg, mLiftCmd)) {
                mIsLiftCommand = true;
                return CPSComplete;
            }
            if (parseProjectorCommand(arg, mProjectorCmd)) {
                mIsProjectorCommand = true;
                return CPSComplete;
            }
            return CPSInvalidArgument;
        }

        virtual CmdExecStatus completeCommand(bool expectCommand) 
        {
            if (mIsLiftCommand) {
                Projector.moveProjector(mLiftCmd);
            }
            if (mIsProjectorCommand) {
                Projector.sendMode(mProjectorCmd);
            }
            return CmdExecStatus::CESOK;
        }
};

void onProjectorStatus(bool switchState, bool powerOn, bool hasPowerStatus, bool isPowering) {

    Serial.printf("[Projector] Status: Power %s%s, Switch %s\n", hasPowerStatus ? strBool(powerOn) : "-", 
                                                                 isPowering ? " [powering on/off]" : "",
                                                                 strBool(switchState)
                                                            );
}

void onProjectorModeChange(ProjectorCommand mode) {
    Keypad.sendStatus();
}

void checkWiFiConnection()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("[WiFi] Not connected. Reconnecting .. ");
        WiFi.disconnect();
        if (WiFi.reconnect()) {
            Serial.println("successfull.");
        } else {
            Serial.println("failed!");
        }
    }
}

void checkMQTTConnection() 
{
    if (WiFi.status() == WL_CONNECTED && !mqttClient.connected())
    {
        mqttClient.connect();
    }
}

void setup() {
    settings.begin();

    cmdline.begin();
    cmdline.addCommand("status", new StatusParser());
    cmdline.addCommand("led", new LEDParser());
    cmdline.addCommand("wifi", new WiFiParser());
    cmdline.addCommand("mqtt", new MQTTParser());
    cmdline.addCommand("screen", new ScreenParser());
    cmdline.addCommand("projector", new ProjectorParser());

    LEDs.begin();

    Projector.setStatusCallback(onProjectorStatus);
    Projector.setModeChangeCallback(onProjectorModeChange);
    Projector.begin();

    Keypad.begin();

    WiFi.mode(WIFI_STA);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(settings.getHostname().c_str());
    WiFi.begin(settings.getWiFiSSID(), settings.getWiFiPassword());

    mqttClient.setup();
}

void loop() {
    Keypad.loop();
    cmdline.loop();
    mqttClient.loop();
    LEDs.loop();
    Projector.loop();

    EVERY_N_SECONDS( 20 ) {
        checkWiFiConnection();
        checkMQTTConnection();
    }
}
