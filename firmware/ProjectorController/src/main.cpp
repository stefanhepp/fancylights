/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Main loop implementation.
 *
 * Copyright 2025 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */

#include <Arduino.h>

#include <commands.h>

#include <SoftwareSerial.h>

#include "config.h"

static const uint8_t IRQ_BUTTONS = 0;

SoftwareSerial projectorSerial(PIN_SW_RXD, PIN_SW_TXD);

static const int BUF_SIZE = 4;

ProjectorOpcode LastCommand;

void sendKeypadStatus() {
    uint8_t switchStatus = digitalRead(PIN_ENDSTOP) ? 0 : 1;

    Serial.write(ProjectorOpcode::POP_STATUS);
    Serial.write(switchStatus);
}

void processSerial(uint8_t data) {
    // Checking for command byte
    if ((data & 0xA0) == 0xA0) {
        LastCommand = (ProjectorOpcode)data;
        if (data == ProjectorOpcode::POP_STATUS) {
            sendKeypadStatus();
        }
    } else {
        if (LastCommand == POP_MODE) {
            switch ((ProjectorCommand) data) {
                case ProjectorCommand::PROJECTOR_OFF:
                    projectorSerial.write("* 0 IR 002\r");
                    break;
                case ProjectorCommand::PROJECTOR_ON:
                    // Alternative code: "OKOKOKOKOK\r"
                    projectorSerial.write("* 0 IR 001\r");
                    break;
                case ProjectorCommand::PROJECTOR_NORMAL:
                    // 3D Off
                    projectorSerial.write("* 0 IR 057\r");
                    break;
                case ProjectorCommand::PROJECTOR_3D:
                    // 3D On
                    projectorSerial.write("* 0 IR 056\r");
                    // 3D Side-by-side Half
                    projectorSerial.write("* 0 IR 062\r");
                    // 3D L/R Invert
                    projectorSerial.write("* 0 IR 065\r");
                    break;
                default:
                    // not implemented
                    break;
            }
        }
    }
}

void setup() {
    // Enable pullups for unconnected pins
    pinMode(PIN_PB0, INPUT_PULLUP);
    pinMode(PIN_PB2, INPUT_PULLUP);
    pinMode(PIN_PB3, INPUT_PULLUP);
    pinMode(PIN_PB4, INPUT_PULLUP);
    pinMode(PIN_PB5, INPUT_PULLUP);
    pinMode(PIN_PB6, INPUT_PULLUP);
    pinMode(PIN_PB7, INPUT_PULLUP);
    pinMode(PIN_PC0, INPUT_PULLUP);
    pinMode(PIN_PC1, INPUT_PULLUP);
    pinMode(PIN_PC3, INPUT_PULLUP);
    pinMode(PIN_PC4, INPUT_PULLUP);
    pinMode(PIN_PC5, INPUT_PULLUP);
    pinMode(PIN_PC6, INPUT_PULLUP);
    pinMode(PIN_PD4, INPUT_PULLUP);
    pinMode(PIN_PD5, INPUT_PULLUP);
    pinMode(PIN_PD7, INPUT_PULLUP);

    pinMode(PIN_ENDSTOP, INPUT_PULLUP);

    pinMode(PIN_SERVO_ENABLE, OUTPUT);
    pinMode(PIN_SERVO_PWM,    OUTPUT);
    digitalWrite(PIN_SERVO_ENABLE, HIGH);
    digitalWrite(PIN_SERVO_PWM,    LOW);

    Serial.begin(UART_SPEED_CONTROLLER);
    projectorSerial.begin(UART_SPEED_PROJECTOR);
}

void loop() {
    while (Serial.available()) {
        uint8_t data = Serial.read();
        processSerial(data);
    }

}
