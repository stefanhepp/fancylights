/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * RGB LED driver.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <inttypes.h>

#include "Settings.h"

static const uint8_t NUM_LEDS = 5;

static const uint8_t LED_R = 0;
static const uint8_t LED_G = 1;
static const uint8_t LED_B = 2;
static const uint8_t LED_LAMP1 = 3;
static const uint8_t LED_LAMP2 = 4;

class LEDDriver {
    private:
        Settings &mSettings;

        uint8_t mRGB[3];
        int16_t mHSV[3];
        uint8_t mIntensity[NUM_LEDS];

        uint8_t mLightMode = 0;
        uint8_t mLightIntensity = 255;
        uint8_t mDimmedIntensity = 50;

        uint8_t mRGBMode = 0;

        uint8_t mPWMCounter;
        uint8_t mLastCounter;

        int mCycleCounter = 0;

        void recalculate();

        void hsvToRgb();

    public:
        explicit LEDDriver(Settings &settings);

        uint8_t lightMode() const { return mLightMode; }

        uint8_t rgbMode() const { return mRGBMode; }

        uint8_t intensity() const { return mLightIntensity; }

        uint8_t dimmedIntensity() const { return mDimmedIntensity; }

        uint8_t getColor(int index) const { return mRGB[index]; }

        uint8_t getHSV(int component) const { return mHSV[component]; }


        void setLightMode(uint8_t mode);

        void setRGBMode(uint8_t mode);

        void setIntensity(uint8_t intensity);

        void setDimmedIntensity(uint8_t intensity);

        void setHSV(uint8_t hue, uint8_t saturation, uint8_t value);

        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        /**
         * Update LED PWM status.
         * 
         * Return true if PWM counter wrapped around (TOP was reached).
         */
        bool updateLEDs();

        uint8_t getLEDStatus(int index) { 
            return mIntensity[index] > 0 && mPWMCounter <= mIntensity[index] ? 1 : 0; 
        }
};
