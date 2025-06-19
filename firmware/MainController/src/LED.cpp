/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * LED intensity and mode implementation.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "LED.h"

#include <commands.h>

#include <FastLED.h>

#include "config.h"


LEDDriver::LEDDriver(Settings &settings)
: mSettings(settings)
{
    for (uint8_t i = 0; i < NUM_LAMPS; i++) {
        mIntensity[i] = 0;
    }
}

void LEDDriver::updateIntensity()
{
    if (mEnableLamps) {
        uint8_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : mLightIntensity;

        mIntensity[LED_LAMP1] = intensity;
        mIntensity[LED_LAMP2] = intensity;
    } else {
        mIntensity[LED_LAMP1] = 0;
        mIntensity[LED_LAMP2] = 0;
    }

    if (mEnableLEDStrip) {
        uint16_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : 256;
        
        FastLED.setBrightness(intensity);
    }

    analogWrite(PIN_LAMP1, mIntensity[LED_LAMP1]);
    analogWrite(PIN_LAMP2, mIntensity[LED_LAMP2]);
}

void LEDDriver::updateLEDs(bool force) {
    if (mEnableLEDStrip) {
        switch (mRGBMode) {
            case RGBMode::RGB_ON:
                break;
            case RGBMode::RGB_CYCLE:

                break;
            case RGBMode::RGB_FIRE:

                break;
        }
    }
}

void LEDDriver::enableLamps(bool enabled)
{
    mEnableLamps = enabled;
    mSettings.setLampEnabled(enabled);
    updateIntensity();
}

void LEDDriver::enableLEDStrip(bool enabled)
{
    mEnableLEDStrip = enabled;
    mSettings.setLEDStripEnabled(enabled);

    digitalWrite(PIN_RGB_PWR, mEnableLEDStrip ? HIGH : LOW);

    updateIntensity();
    updateLEDs(true);
}

void LEDDriver::setRGBMode(RGBMode mode)
{
    mRGBMode = mode;
    mSettings.setRGBMode(mode);
    updateLEDs(true);
}

void LEDDriver::setIntensity(uint8_t value)
{
    mLightIntensity = value;
    mSettings.setIntensity(value);
    updateIntensity();
}

void LEDDriver::setDimmedIntensity(uint8_t value)
{
    mDimmedIntensity = value;
    mSettings.setDimmedIntensity(value);
    updateIntensity();
}

void LEDDriver::setHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
    mHSV.setHSV(hue, saturation, value);
    mSettings.setHSV(hue, saturation, value);
    updateLEDs(true);
}

void LEDDriver::begin()
{
    analogWriteFrequency(5000);
    analogWriteResolution(8);

    pinMode(PIN_RGB_DATA, OUTPUT);
    pinMode(PIN_RGB_PWR, OUTPUT);

    mEnableLamps = mSettings.isLampEnabled();
    mEnableLEDStrip = mSettings.isLEDStripEnabled();

    digitalWrite(PIN_RGB_PWR, mEnableLEDStrip ? HIGH : LOW);

    mLightIntensity = mSettings.intensity();
    mDimmedIntensity = mSettings.dimmedIntensity();

    mRGBMode = mSettings.rgbMode();

    mHSV = mSettings.getHSV();

    FastLED.addLeds<WS2815, PIN_RGB_DATA, GRB>(mLEDs, NUM_LEDS).setCorrection( TypicalLEDStrip );

    updateIntensity();
    updateLEDs(true);
}

void LEDDriver::loop()
{
    EVERY_N_MILLISECONDS( 20 ) {
        if (mRGBMode == RGBMode::RGB_CYCLE) {
            mHSV[0]++;
            if (mHSV[0] > 255) {
                mHSV[0] = 0;
            }
            updateLEDs();
        }
        else if (mRGBMode == RGBMode::RGB_FIRE) {

        }

        FastLED.show();
    }

}
