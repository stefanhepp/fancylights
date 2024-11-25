/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Maintains the state of the light controller.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "LightState.h"

#include <inttypes.h>
#include <avr/io.h>

#include <avrlib.h>

#include "commands.h"

LightState::LightState()
{
    mLightMode = 0;
    mLightIntensity = 255;
    mRGBMode = RGB_ON;
    mRGB[0] = 255;
    mRGB[1] = 255;
    mRGB[2] = 255;
    mProjectorMode = PROJECTOR_OFF;
}

void LightState::begin()
{
}

void LightState::toggleLight()
{
    mLightMode = mLightMode ? 0 : 0x03;
}

void LightState::toggleProjector()
{
    mProjectorMode = mProjectorMode != PROJECTOR_OFF ? PROJECTOR_OFF : PROJECTOR_ON;
}

void LightState::changeIntensity(int delta)
{
    mLightIntensity += delta;
    if (mLightIntensity < 0) {
        mLightIntensity = 0;
    }
    if (mLightIntensity > 255) {
        mLightIntensity = 255;
    }
}

void LightState::changeRGBSaturation(int delta)
{

}

void LightState::changeRGBHue(int delta)
{
    
}

void LightState::setRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    mRGB[0] = red;
    mRGB[1] = green;
    mRGB[2] = blue;
}
