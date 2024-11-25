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

        uint8_t mLightIntensity;

        uint8_t mRGBMode;

        uint8_t mRGB[3];

        uint8_t mProjectorMode;

    public:
        explicit LightState();

        /**
         * Initialize all pins and routines.
         **/
        void begin();


        void toggleLight();

        void toggleProjector();


        void setLightMode(uint8_t mode) { mLightMode = mode; }

        void setIntensity(uint8_t intensity) { mLightIntensity = intensity; }

        void setRGB(uint8_t red, uint8_t green, uint8_t blue);

        void setRGBMode(uint8_t mode) { mRGBMode = mode; }

        void setProjectorMode(uint8_t mode) { mProjectorMode = mode; }


        uint8_t lightMode() const { return mLightMode; }

        uint8_t lightIntensity() const { return mLightIntensity; }

        uint8_t rgb(int color) const { return mRGB[color]; }

        uint8_t rgbMode() const { return mRGBMode; }

        uint8_t projectorMode() const { return mProjectorMode; }
};
