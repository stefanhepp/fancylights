/*
 * @project     FancyController
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
#include "LED.h"
#include "Settings.h"
#include "ProjectorController.h"

Settings settings;
LEDDriver LEDs(settings);
ProjectorController Projector(settings);

static const int UART_BUFFER_SIZE = 16;
static uint8_t UARTBuffer[UART_BUFFER_SIZE];
static uint8_t UARTBufferLength = 0;

uint8_t CntStatusTimeout = 0;

void sendKeypadStatus()
{
    Serial.write(CMD_HEADER | CMD_READ_STATUS);
    Serial.write(LEDs.lightMode());
    Serial.write(LEDs.intensity());
    Serial.write(LEDs.dimmedIntensity());
    Serial.write(Projector.mode());
    Serial.write(LEDs.rgbMode());
    Serial.write(LEDs.getHSV(0));
    Serial.write(LEDs.getHSV(1));
    Serial.write(LEDs.getHSV(2));

    Serial.write(LEDs.getColor(0));
    Serial.write(LEDs.getColor(1));
    Serial.write(LEDs.getColor(2));

}

void onProjectorStatus(bool powerOn) {
    CntStatusTimeout = 0;
    sendKeypadStatus();
}

void processKeypadCommand(uint8_t data)
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
            }
            break;
        case CMD_DIMMED_INTENSITY:
            if (UARTBufferLength >= 2) {
                LEDs.setDimmedIntensity(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_HSV_COLOR:
            if (UARTBufferLength >= 4) {
                LEDs.setHSV(UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_LIGHT_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.setLightMode(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_RGB_MODE:
            if (UARTBufferLength >= 2) {
                LEDs.setRGBMode(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_SCREEN:
            if (UARTBufferLength >= 2) {
                Projector.moveScreen(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_PROJECTOR_MODE:
            if (UARTBufferLength >= 2) {
                Projector.setMode(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_PROJECTOR_LIFT:
            if (UARTBufferLength >= 2) {
                Projector.moveProjector(UARTBuffer[1]);
                UARTBufferLength = 0;
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
    // Enable pullups for unconnected pins
    pinMode(PIN_PB3, INPUT_PULLUP);
    pinMode(PIN_PB4, INPUT_PULLUP);
    pinMode(PIN_PB5, INPUT_PULLUP);
    pinMode(PIN_PC4, INPUT_PULLUP);
    pinMode(PIN_PC5, INPUT_PULLUP);
    pinMode(PIN_PC6, INPUT_PULLUP);
    pinMode(PIN_PD2, INPUT_PULLUP);
    pinMode(PIN_PD4, INPUT_PULLUP);
    pinMode(PIN_PD7, INPUT_PULLUP);

    LEDs.begin();

    Projector.setStatusCallback(onProjectorStatus);
    Projector.begin();

    Serial.begin(UART_SPEED_CONTROLLER);
}

void loop() {

    bool wrapAround = LEDs.updateLEDs();

    digitalWrite(PIN_R, !LEDs.getLEDStatus(LED_R));
    digitalWrite(PIN_G, !LEDs.getLEDStatus(LED_G));
    digitalWrite(PIN_B, !LEDs.getLEDStatus(LED_B));
    digitalWrite(PIN_LAMP1, !LEDs.getLEDStatus(LED_LAMP1));
    digitalWrite(PIN_LAMP2, !LEDs.getLEDStatus(LED_LAMP2));

    if (wrapAround) {
        while (Serial.available()) {
            int data = Serial.read();

            processKeypadCommand(data);
        }

        if (CntStatusTimeout > 0) {
            CntStatusTimeout--;
            if (CntStatusTimeout == 0) {
                sendKeypadStatus();
            }
        }

        Projector.loop();
    }
}
