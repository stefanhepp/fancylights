/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Input encoding and processing routines.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <inttypes.h>

#include "config.h"

static const uint8_t NUM_BUTTONS = 4*4;

// Number of ticks for long press, in 6*1.5 us per tick, about 2 seconds.
static const uint8_t LONG_PRESS_DURATION = 240;

using KeyPressCallback = void(*)(uint8_t btn, bool longPress);

class Keypad
{
    private:
        // callback for key changes
        KeyPressCallback mKeyPressCallback = nullptr;

        uint8_t mButtons[NUM_BUTTONS];

        uint8_t getBtnNumber(uint8_t row, uint8_t pin);

        void readLine(const uint8_t row);
    public:
        explicit Keypad();

        void setPressEvent(KeyPressCallback callback) { mKeyPressCallback = callback; }

        /**
         * Initialize all pins and routines.
         **/
        void begin();

        /** 
         * poll input ports for changes, call handler on changed notes.
         **/
        void poll();
};
