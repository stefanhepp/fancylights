/*
 * @project     FancyLights
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

#include <chsv.h>
#include <crgb.h>

#include <commands.h>

#include "Settings.h"

static const uint8_t NUM_LAMPS = 2;

static const uint8_t LED_LAMP1 = 0;
static const uint8_t LED_LAMP2 = 1;

static const uint8_t NUM_LEDS = 60 * 4;

class LEDDriver {
    private:
        Settings &mSettings;

        CHSV    mHSV;
    
        uint8_t mIntensity[NUM_LAMPS];
        CRGB    mLEDs[NUM_LEDS];

        bool    mEnableLamps = false;
        bool    mEnableLEDStrip = false;

        uint8_t mLightIntensity = 255;
        uint8_t mDimmedIntensity = 50;

        RGBMode mRGBMode = RGB_ON;

        void updateIntensity();

        void updateLEDs(bool force = false);

    public:
        explicit LEDDriver(Settings &settings);


        bool    isLampEnabled() { return mEnableLamps; }

        bool    isLEDStripEnabled() { return mEnableLEDStrip; }

        uint8_t intensity() const { return mLightIntensity; }

        uint8_t dimmedIntensity() const { return mDimmedIntensity; }


        RGBMode rgbMode() const { return mRGBMode; }

        const CHSV &getHSV() const { return mHSV; }


        void enableLamps(bool enabled);

        void enableLEDStrip(bool enabled);

        void setIntensity(uint8_t intensity);

        void setDimmedIntensity(uint8_t intensity);


        void setRGBMode(RGBMode mode);

        void setHSV(uint8_t hue, uint8_t saturation, uint8_t value);

        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        /**
         * Update LED PWM status.
         */
        void loop();
};
