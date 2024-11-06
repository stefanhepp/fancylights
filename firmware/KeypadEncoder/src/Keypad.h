/*
 * @project     PistonController
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

// We use 3 bytes = 24 bits per keyboard
static const uint8_t BITS_PER_KEYBOARD = 24;
static const uint8_t NUM_BUTTONS = BITS_PER_KEYBOARD * 2;

// Number of ticks for long press, in 6*1.5 us per tick, about 2 seconds.
static const uint8_t LONG_PRESS_DURATION = 120;

class Pistons
{
    private:
        // callback for key changes
        void (*mPressEvent)(uint8_t kbd, uint8_t btn, bool longPress) = nullptr;

        uint8_t mLEDs[6];

        uint8_t mButtons[NUM_BUTTONS];

        uint8_t mNumKeyboards;

        uint8_t getBtnNumber(uint8_t addr, uint8_t pin);


        uint8_t getLEDNumber(uint8_t addr, uint8_t pin);

        uint8_t getBitValue(const uint8_t* array, uint8_t btnNumber);

        void emitButtonPress(uint8_t btn, bool longPress);

        void readLine(const uint8_t line);

        void writeLEDs(const uint8_t line);
    public:
        explicit Pistons();

        void setPressEvent(void (*handler)(uint8_t kbd, uint8_t btn, bool longPress) );

        void setLEDs(uint8_t kbd, uint8_t mask1, uint8_t mask2, uint8_t mask3, uint8_t mask4);

        /**
         * Reset the encoder.
         */
        void reset();

        /**
         * Initialize all pins and routines.
         **/
        void begin(uint8_t numKeyboards);

        /** 
         * poll input ports for changes, call handler on changed notes.
         **/
        void poll();
};
