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
    mDimmedIntensity = 50;
    mRGBMode = RGB_ON;
    mProjectorMode = PROJECTOR_OFF;
    // Don't initialize mHSV to save space. Will be initialzed by first received status message.
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
    // Adapt RGB light intensity to main intensity when set via keypad
    mHSV[2] = mLightIntensity;
}

void LightState::changeHSVSaturation(int delta)
{
    mHSV[1] += delta;
    if (mHSV[1] < 0) {
        mHSV[1] = 0;
    }
    if (mHSV[1] > 255) {
        mHSV[1] = 255;
    }
}

void LightState::changeHSVHue(int delta)
{
    mHSV[0] += delta;
    if (mHSV[0] < 0) {
        mHSV[0] += 255;
    }
    if (mHSV[0] > 255) {
        mHSV[0] -= 255;
    }    
}

void LightState::setHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
    mHSV[0] = hue;
    mHSV[1] = saturation;
    mHSV[2] = value;
}
