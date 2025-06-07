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

class StatusParser: public CommandParser
{
    public:
        StatusParser() {}

        virtual void printArguments() { 
            Serial.print("all");
        }

        virtual CmdErrorCode startCommand(const char* cmd) {
            Serial.printf("Light Mode: %hhu\n", LEDs.lightMode());
            Serial.printf("Light Intensity: %hhu\n", LEDs.intensity());

            return CmdErrorCode::CmdOK;
        }
};

void sendStatus()
{
    keypadSerial.write(CMD_HEADER | CMD_READ_STATUS);
    keypadSerial.write(LEDs.lightMode());
    keypadSerial.write(LEDs.intensity());
    keypadSerial.write(LEDs.dimmedIntensity());
    keypadSerial.write(Projector.mode());
    keypadSerial.write(LEDs.rgbMode());
    keypadSerial.write(LEDs.getHSV(0));
    keypadSerial.write(LEDs.getHSV(1));
    keypadSerial.write(LEDs.getHSV(2));

    keypadSerial.write(LEDs.getColor(0));
    keypadSerial.write(LEDs.getColor(1));
    keypadSerial.write(LEDs.getColor(2));
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
                Serial.printf("Set light intensity: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_DIMMED_INTENSITY:
            if (UARTBufferLength >= 2) {
                LEDs.setDimmedIntensity(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("Set dimmed intensity: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_HSV_COLOR:
            if (UARTBufferLength >= 4) {
                LEDs.setHSV(UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
                UARTBufferLength = 0;
                Serial.printf("Set HSV: %hhu %hhu %hhu\n", UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
            }
            break;
        case CMD_LIGHT_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.setLightMode(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("Set light mode: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_RGB_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.setRGBMode(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("Set RGB mode: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_SCREEN:
            if (UARTBufferLength >= 2) {
                Projector.moveScreen(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("Move Screen: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_PROJECTOR_MODE:
            if (UARTBufferLength >= 2) {
                Projector.setMode(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("Set projector mode: %hhu\n", UARTBuffer[1]);
            }
            break;
        case CMD_PROJECTOR_LIFT:
            if (UARTBufferLength >= 2) {
                Projector.moveProjector(UARTBuffer[1]);
                UARTBufferLength = 0;
                Serial.printf("Lift projector: %hhu\n", UARTBuffer[1]);
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

    LEDs.begin();

    Projector.setStatusCallback(onProjectorStatus);
    Projector.begin();

    keypadSerial.begin(UART_SPEED_CONTROLLER, SERIAL_8N1, PIN_KP_RXD, PIN_KP_TXD);
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

    delayMicroseconds(1000);
}
