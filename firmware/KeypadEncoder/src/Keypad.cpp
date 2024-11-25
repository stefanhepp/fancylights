/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Input processing routines.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "Keypad.h"

#include <inttypes.h>
#include <avr/io.h>

#include <avrlib.h>

#include "config.h"

static const uint8_t PIN_ROWS[4] = {PIN_K1, PIN_K2, PIN_K3, PIN_K4};
static const uint8_t PIN_COLS[4] = {PIN_K5, PIN_K6, PIN_K7, PIN_K8}; 

Keypad::Keypad()
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        mButtons[i] = 0;
    }
}

uint8_t Keypad::getBtnNumber(uint8_t row, uint8_t pin)
{
    return row * 4 + pin;
}

void Keypad::begin()
{
    for (int i = 0; i < 4; i++) {
        pinMode(PIN_ROWS[i], OUTPUT);
        pinMode(PIN_COLS[i], INPUT_PULLUP);

        digitalWrite(PIN_ROWS[i], LOW);
    }
}

void Keypad::readLine(const uint8_t row) {
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t btn = getBtnNumber(row, i);        
        uint8_t pressed = !digitalRead(PIN_COLS[i]);

        if (pressed) {
            // Increase counter, to measure duration of button press
            if (mButtons[btn] < 0xFF) {
                mButtons[btn]++;
                if (mButtons[btn] > LONG_PRESS_DURATION) {
                    if (mKeyPressCallback) {
                        mKeyPressCallback(btn, true);
                    }
                    mButtons[btn] = 0xFF;
                }
            }
        } else {
            if (mButtons[btn] > 0) {
                // Button was pressed, is now released
                if (mButtons[btn] != 0xFF) {
                    if (mKeyPressCallback) {
                        mKeyPressCallback(btn, false);
                    }
                }
                mButtons[btn] = 0;
            }
        }
    }
}

void Keypad::poll()
{
    uint8_t row;

    for (row = 0; row < 4; row++) {
        // select new line
        digitalWrite(PIN_ROWS[row], LOW);

        delayMicroseconds(100);

        readLine(row);

        digitalWrite(PIN_ROWS[row], HIGH);
    }

    delay(2);
}
