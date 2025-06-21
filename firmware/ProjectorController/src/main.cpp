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

#include "config.h"

static const uint8_t IRQ_BUTTONS = 0;

void sendStatus() {
    uint8_t switchStatus = digitalRead(PIN_ENDSTOP);

    Serial.write(ProjectorOpcode::POP_STATUS);
    Serial.write(switchStatus);
}

void processSerial(uint8_t data) {
    if (data == ProjectorOpcode::POP_STATUS) {
        sendStatus();
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

    pinMode(PIN_SW_RXD, INPUT_PULLUP);
    pinMode(PIN_SW_TXD, OUTPUT);
    digitalWrite(PIN_SW_TXD, HIGH);

    Serial.begin(UART_SPEED_PROJECTOR);
}

void loop() {
    while (Serial.available()) {
        uint8_t data = Serial.read();
        processSerial(data);
    }

}
