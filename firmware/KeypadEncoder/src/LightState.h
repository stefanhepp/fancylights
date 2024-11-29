/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Maintains the state of the light controller.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <inttypes.h>

#include "config.h"

class LightState
{
    private:
        uint8_t mLightMode;

        // Int to make math ops easier
        int16_t mLightIntensity;

        uint8_t mDimmedIntensity;

        uint8_t mRGBMode;

        int16_t mHSV[3];

        uint8_t mProjectorMode;

    public:
        explicit LightState();

        /**
         * Initialize all pins and routines.
         **/
        void begin();


        void toggleLight();

        void toggleProjector();

        void changeIntensity(int delta);

        void changeHSVSaturation(int delta);

        void changeHSVHue(int delta);
        

        void setLightMode(uint8_t mode) { mLightMode = mode; }

        void setIntensity(uint8_t intensity) { mLightIntensity = intensity; }

        void setDimmedIntensity(uint8_t intensity) { mDimmedIntensity = intensity; }

        void setHSV(uint8_t hue, uint8_t saturation, uint8_t value);

        void setRGBMode(uint8_t mode) { mRGBMode = mode; }

        void setProjectorMode(uint8_t mode) { mProjectorMode = mode; }


        uint8_t lightMode() const { return mLightMode; }

        uint8_t lightIntensity() const { return mLightIntensity; }

        uint8_t dimmedIntensity() const { return mDimmedIntensity; }

        uint8_t hsv(int component) const { return mHSV[component]; }

        uint8_t rgbMode() const { return mRGBMode; }

        uint8_t projectorMode() const { return mProjectorMode; }
};
