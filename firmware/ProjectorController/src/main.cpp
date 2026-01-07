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

int PosResponse = -1;
int StatusTimeout = 0;

ProjectorOpcode LastCommand;


void requestProjectorStatus() 
{
    // start reading and set timeout
    PosResponse = 0;
    StatusTimeout = 10;

    // request status from projector
    projectorSerial.write("* 0 Lamp ?\r");
}

void sendProjectorStatus(bool hasPowerStatus = false, bool powerOn = false) {
    uint8_t switchStatus = digitalRead(PIN_ENDSTOP) ? 0 : 1;

    Serial.write(ProjectorOpcode::POP_STATUS);
    Serial.write(hasPowerStatus << 2 | powerOn << 1 | switchStatus);
}

void processProjectorData(uint8_t data) {
    if (PosResponse < 0) {
        // not expecting data
        return;
    }

    if (PosResponse < 5) {
        if (data == "Lamp "[PosResponse]) {
            PosResponse++;
        } else {
            // invalid response, send status and stop processing
            sendProjectorStatus();
            StatusTimeout = 0;
            PosResponse = -1;
        }
    }
    if (PosResponse == 5) {
        if (data != '1' && data != '0') {
            // invalid response, send status and stop processing
            sendProjectorStatus();
            StatusTimeout = 0;
            PosResponse = -1;
            return;
        }

        // got response, send status and stop processing
        sendProjectorStatus(true, data == '1');
        StatusTimeout = 0;
        PosResponse = -1;
    }
}

void processSerial(uint8_t data) {
    // Checking for opcode byte
    if ((data & 0xA0) == 0xA0) {
        // received opcode byte
        LastCommand = (ProjectorOpcode)data;
        // No additional data for status request, handle immediately
        if (data == ProjectorOpcode::POP_STATUS) {
            requestProjectorStatus();
        }
    } else {
        // process command byte after opcode byte
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

    while (projectorSerial.available()) {
        uint8_t data = projectorSerial.read();
        processProjectorData(data);
    }

    if (StatusTimeout > 0) {
        StatusTimeout--;
        if (StatusTimeout == 0) {
            // Timeout, send what we have
            sendProjectorStatus(false);
        }
    }
}
