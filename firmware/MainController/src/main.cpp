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

Settings settings;
CommandLine cmdline;
LEDDriver LEDs(settings);
ProjectorController Projector(settings);

HardwareSerial keypadSerial(UART_KEYPAD);

static const int UART_BUFFER_SIZE = 16;
static uint8_t UARTBuffer[UART_BUFFER_SIZE];
static uint8_t UARTBufferLength = 0;

uint8_t CntStatusTimeout = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

class StatusParser: public CommandParser
{
    public:
        StatusParser() {}

        virtual void printArguments() { 
            Serial.print("all");
        }

        virtual CmdErrorCode startCommand(const char* cmd) {
            Serial.printf("Enable Lamp: %hhu, LED Strip: %hhu\n", LEDs.isLampEnabled(), LEDs.isLEDStripEnabled());
            Serial.printf("Light Intensity: %hhu\n", LEDs.intensity());
            Serial.printf("Dimmed Intensity: %hhu\n", LEDs.dimmedIntensity());
            Serial.printf("RGB Strip Mode: %hhu\n", LEDs.rgbMode());
            Serial.printf("Projector Mode: %hhu\n", Projector.mode());
            Serial.printf("HSV: %hhu %hhu %hhu\n", LEDs.getHSV().hue, LEDs.getHSV().sat, LEDs.getHSV().val);

            return CmdErrorCode::CmdOK;
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

void onProjectorStatus(bool powerOn) {
    CntStatusTimeout = 0;
    sendStatus();
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
                Serial.printf("[Kbd] Enable lamp: %hhu, LED strip: %hhu\n", (UARTBuffer[1] & 0x01), (UARTBuffer[1] & 0x02));
            }
            break;
        case CMD_RGB_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.setRGBMode((RGBMode) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Set RGB mode: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_SCREEN:
            if (UARTBufferLength >= 2) {
                Projector.moveScreen((LiftCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Move Screen: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_PROJECTOR_MODE:
            if (UARTBufferLength >= 2) {
                Projector.setMode((ProjectorCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Set projector mode: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_PROJECTOR_LIFT:
            if (UARTBufferLength >= 2) {
                Projector.moveProjector((LiftCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("[Kbd] Lift projector: %hhu\n", UARTBuffer[1]);
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

void setup() {
    settings.begin();

    cmdline.begin();
    cmdline.addCommand("status", new StatusParser());

    LEDs.begin();

    Projector.setStatusCallback(onProjectorStatus);
    Projector.begin();

    keypadSerial.begin(UART_SPEED_CONTROLLER, SERIAL_8N1, PIN_KP_RXD, PIN_KP_TXD);

    WiFi.begin("HomeLan", "");

    mqttClient.setServer("mqtt.home", 1883);
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

    EVERY_N_SECONDS( 1 ) {
        if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {

        }
    }
}
