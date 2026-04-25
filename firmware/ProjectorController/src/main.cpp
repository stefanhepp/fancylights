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
unsigned long StatusTimeout = 0;

ProjectorOpcode LastCommand;

ProjectorOpcode LockState = ProjectorOpcode::POP_ACK;

bool LastSwitchState = false;

bool isSwitchPressed()
{
    return digitalRead(PIN_ENDSTOP) == LOW;
}

void requestProjectorStatus() 
{
    // start reading and set timeout
    PosResponse = 0;
    StatusTimeout = millis() + 800;

    // request status from projector
    projectorSerial.write("* 0 Lamp ?\r");
}

void moveServo(bool lock) {
    digitalWrite(PIN_SERVO_ENABLE, LOW);

    // generate PWM signal for about 1 second
    for (int i = 0; i < 50; i++) {
        digitalWrite(PIN_SERVO_PWM, HIGH);
        delayMicroseconds(lock ? 1900 : 1000);
        digitalWrite(PIN_SERVO_PWM, LOW);
        delay(18);
    }

    digitalWrite(PIN_SERVO_ENABLE, HIGH);
}

void confirmLock() {
    Serial.write(ProjectorOpcode::POP_ACK);
    LockState = ProjectorOpcode::POP_ACK;
}

void sendProjectorStatus(bool hasPowerStatus = false, bool powerOn = false) {
    uint8_t switchStatus = isSwitchPressed() ? 1 : 0;

    Serial.write(ProjectorOpcode::POP_STATUS);
    Serial.write(hasPowerStatus << 2 | powerOn << 1 | switchStatus);

    // clear timeout
    StatusTimeout = 0;
    PosResponse = -1;
}

void processProjectorData(uint8_t data) {
    if (PosResponse < 0) {
        // not expecting data
        return;
    }

    if (PosResponse < 10) {
        if (PosResponse > 0 && PosResponse < 3) {
            // This is some counter, ignore
            PosResponse++;
        }
        else if (data == "*000\rLamp "[PosResponse]) {
            PosResponse++;
        }
        else {
            // invalid response, send status and stop processing
            sendProjectorStatus();
        }
    }
    else if (PosResponse == 10) {
        if (data != '1' && data != '0') {
            // invalid response, send status and stop processing
            sendProjectorStatus();
        }
        else {
            // got response, send status and stop processing
            sendProjectorStatus(true, data == '1');
        }
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
        else if (data == ProjectorOpcode::POP_LOCK) {
            LockState = ProjectorOpcode::POP_LOCK;
        }
        else if (data == ProjectorOpcode::POP_UNLOCK) {
            LockState = ProjectorOpcode::POP_UNLOCK;
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
                case ProjectorCommand::PROJECTOR_VR:
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
            // Stop processing
            LastCommand = ProjectorOpcode::POP_ACK;
        }
    }
}

void setup()
{
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
        // Check if timeout is reached
        unsigned long now = millis();
        if ((StatusTimeout < now && (now - StatusTimeout) < 2000)
         || (now < 2000 && StatusTimeout > 2000))
        {
            // Timeout, send what we have
            sendProjectorStatus(false);
        }
    }

    bool endstopPressed = isSwitchPressed();
    if (!LastSwitchState && endstopPressed) {
        // Switch is pressed, send status update
        sendProjectorStatus(false);
    }
    LastSwitchState = endstopPressed;

    if (LockState == ProjectorOpcode::POP_UNLOCK) {
        if (endstopPressed) {
            // Endstop reached, release lock
            moveServo(false);
            confirmLock();
        }
    } else if (LockState == ProjectorOpcode::POP_LOCK) {
        if (endstopPressed) {
            // Endstop reached, engage lock
            moveServo(true);
            confirmLock();
        }
    }
}
