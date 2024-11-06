/*
 * @project     PistonController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Input processing routines.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "Pistons.h"

#include <inttypes.h>
#include <avr/io.h>

#include <avrlib.h>

#include <common_config.h>

#include "config.h"

Pistons::Pistons()
: mNumKeyboards(0)
{
    reset();
}

void Pistons::setPressEvent( void(*handler)(uint8_t kbd, uint8_t btn, bool longPress) ) {
    mPressEvent = handler;
}

uint8_t Pistons::getBtnNumber(uint8_t addr, uint8_t pin)
{
    // reverse the lower and higher 4 pins each
    uint8_t pinOrdered = pin > 3 ? 7 - (pin - 4) : 3 - pin;    

    return pinOrdered * 6 + addr;
}

uint8_t Pistons::getLEDNumber(uint8_t addr, uint8_t pin)
{
    // rotate by 1
    uint8_t pinOrdered = pin == 0 ? 7 : pin - 1;

    return pinOrdered * 6 + addr;
}

uint8_t Pistons::getBitValue(const uint8_t* array, uint8_t btnNumber)
{
    uint8_t idx = btnNumber / 8;
    uint8_t value = array[idx];
    return (value >> (btnNumber % 8)) & 0x01;
}

void Pistons::setLEDs(uint8_t kbd, uint8_t mask1, uint8_t mask2, uint8_t mask3, uint8_t mask4)
{
    mLEDs[kbd*3 + 0] = mask4;
    mLEDs[kbd*3 + 1] = mask3;
    mLEDs[kbd*3 + 2] = mask2;
    if (mNumKeyboards == 1 && kbd == 0) {
        mLEDs[3] = mask1;
    }
}

void Pistons::reset()
{
    for (uint8_t i = 0; i < 6; i++) {
        mLEDs[i] = 0;
    }
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        mButtons[i] = 0;
    }
}

void Pistons::begin(uint8_t numKeyboards)
{
    mNumKeyboards = numKeyboards;

    // init ports (in port without pullups, out ports, all disabled)
    IO_DDR(PORT_IN) = 0x00;
    IO_PORT(PORT_IN) = 0x00;

    IO_DDR(PORT_OUT) = 0xFF;
    IO_PORT(PORT_OUT) = 0x00;

    pinMode(PIN_ADDR1,   OUTPUT);
    pinMode(PIN_ADDR2,   OUTPUT);
    pinMode(PIN_ADDR3,   OUTPUT);

    digitalWrite(PIN_ADDR1, LOW);
    digitalWrite(PIN_ADDR2, LOW);
    digitalWrite(PIN_ADDR3, LOW);
}


void Pistons::emitButtonPress(uint8_t btn, bool longPress) {
    if (mPressEvent) {
        if (mNumKeyboards > 1) {
            // for 2 keyboards, we use 3 bytes per keyboard
            mPressEvent(btn / BITS_PER_KEYBOARD, 
                        btn % BITS_PER_KEYBOARD, 
                        longPress);
        } else {
            mPressEvent(0, btn, longPress);
        }
    }
}

void Pistons::readLine(const uint8_t line) {
    uint8_t in = IO_PIN(PORT_IN);

    for (uint8_t i = 0; i < 8; i++) {
        uint8_t btn = getBtnNumber(line, i);
        uint8_t pressed = (in >> i) & 0x01;

        if (pressed) {
            // Increase counter, to measure duration of button press
            if (mButtons[btn] < 0xFF) {
                mButtons[btn]++;
                if (mButtons[btn] > LONG_PRESS_DURATION) {
                    emitButtonPress(btn, true);
                    mButtons[btn] = 0xFF;
                }
            }
        } else {
            if (mButtons[btn] > 0) {
                // Button was pressed, is now released
                if (mButtons[btn] != 0xFF) {
                    emitButtonPress(btn, false);
                }
                mButtons[btn] = 0;
            }
        }

    }
}

void Pistons::writeLEDs(const uint8_t line)
{
    uint8_t out = 0x00;

    for (uint8_t i = 0; i < 8; i++) {
        uint8_t btn = getLEDNumber(line, i);
        out |= (getBitValue(mLEDs, btn) << i);
    }

    IO_PORT(PORT_OUT) = out;
}

void Pistons::poll()
{
    uint8_t line;

    for (line = 0; line < 6; line++) {
        // clear LEDs before switching
        IO_PORT(PORT_OUT) = 0x00;

        // select new line
        digitalWrite(PIN_ADDR1, (line     ) & 0x01);
        digitalWrite(PIN_ADDR2, (line >> 1) & 0x01);
        digitalWrite(PIN_ADDR3, (line >> 2) & 0x01);

        writeLEDs(line);

        delayMicroseconds(500);

        readLine(line);
    }
}
