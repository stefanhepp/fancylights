/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * LED intensity and mode implementation.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "LED.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <avrlib.h>

#include <commands.h>

#include "config.h"


LEDDriver::LEDDriver(Settings &settings)
: mSettings(settings), mPWMCounter(0), mLastCounter(0)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        mIntensity[i] = 0;
    }
}

void LEDDriver::rgbToHsv()
{
    int Cmax = max(mRGB[0], max(mRGB[1], mRGB[2]));
    int Cmin = min(mRGB[0], min(mRGB[1], mRGB[2]));

    int delta = Cmax - Cmin;

    mHSV[0] = delta == 0 ? 0 :
              Cmax == mRGB[LED_R] ? 43 * (((mRGB[LED_G] - mRGB[LED_B]) / delta) % 6) :
              Cmax == mRGB[LED_G] ? 43 * (((mRGB[LED_B] - mRGB[LED_R]) / delta) + 2 ) :
                                    43 * (((mRGB[LED_R] - mRGB[LED_G]) / delta) + 4 ) ;

    mHSV[1] = Cmax == 0 ? 0 : ((int)delta * 255) / Cmax;
    mHSV[2] = Cmax;
}

void LEDDriver::hsvToRgb()
{
    int C = ((int)mHSV[1] * (int)mHSV[2]) / 255;
    int X = C * (1 - abs((mHSV[0] / 43) % 2 - 1));
    int m = mHSV[2] - C;

    int r,g,b;
    switch (mHSV[0]/43) {
        case 0:
            r = C; g = X; b = 0;
            break;
        case 1:
            r = X; g = C; b = 0;
            break;
        case 2:
            r = 0; g = C; b = X;
            break;
        case 3:
            r = 0; g = X; b = C;
            break;
        case 4:
            r = X; g = 0; b = C;
            break;
        case 5:
            r = C; g = 0; b = X;
            break;
    }

    mRGB[LED_R] = r + m;
    mRGB[LED_G] = g + m;
    mRGB[LED_B] = b + m;
}

void LEDDriver::recalculate()
{
    if (mLightMode | 0x01) {
        uint8_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : mLightIntensity;

        mIntensity[LED_LAMP1] = intensity;
        mIntensity[LED_LAMP2] = intensity;
    } else {
        mIntensity[LED_LAMP1] = 0;
        mIntensity[LED_LAMP2] = 0;
    }

    if (mLightMode | 0x02) {
        int intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : 255;

        mIntensity[LED_R] = (((int)mRGB[LED_R])*intensity)/255;
        mIntensity[LED_G] = (((int)mRGB[LED_G])*intensity)/255;
        mIntensity[LED_B] = (((int)mRGB[LED_B])*intensity)/255;
    }
}

void LEDDriver::setLightMode(uint8_t mode)
{
    mLightMode = mode;
    mSettings.setLightMode(mode);
    recalculate();
}

void LEDDriver::setRGBMode(uint8_t mode)
{
    mRGBMode = mode;
    mSettings.setRGBMode(mode);
    recalculate();
}

void LEDDriver::setIntensity(uint8_t value)
{
    mLightIntensity = value;
    mSettings.setIntensity(value);
    recalculate();
}

void LEDDriver::setDimmedIntensity(uint8_t value)
{
    mDimmedIntensity = value;
    mSettings.setDimmedIntensity(value);
    recalculate();
}

void LEDDriver::setColor(uint8_t red, uint8_t green, uint8_t blue)
{
    mRGB[LED_R] = red;
    mRGB[LED_G] = green;
    mRGB[LED_B] = blue;
    mSettings.setColor(LED_R, red);
    mSettings.setColor(LED_G, green);
    mSettings.setColor(LED_B, blue);
    rgbToHsv();
    recalculate();
}

void LEDDriver::setHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
    mHSV[0] = hue;
    mHSV[1] = saturation;
    mHSV[2] = value;
    mSettings.setColor(3, hue);
    mSettings.setColor(4, saturation);
    mSettings.setColor(5, value);
    hsvToRgb();
    recalculate();
}

void LEDDriver::begin()
{
    mLightMode = mSettings.lightMode();
    mLightIntensity = mSettings.intensity();
    mDimmedIntensity = mSettings.dimmedIntensity();
    mRGBMode = mSettings.rgbMode();
    for (int i = 0; i < 3; i++) {
        mRGB[i] = mSettings.getColor(i);
    }
    rgbToHsv();

    // no output, 8-bit normal PWM
    TCCR2A = 0;

    // set prescaler to CK/256 (~10ms period)
    TCCR2B = (1<<CS22)|(1<<CS21);

    // Enable interrupts
    TIMSK2 = 0;
}

bool LEDDriver::updateLEDs() {
    mLastCounter = mPWMCounter;
    mPWMCounter = TCNT2;
    bool wrapAround = mPWMCounter < mLastCounter;

    if (wrapAround) {
        mCycleCounter = (mCycleCounter + 1) % 4;

        if (mCycleCounter == 0 && mRGBMode == RGBMode::RGB_CYCLE) {
            mHSV[0]++;
            if (mHSV[0] > 255) {
                mHSV[0] = 0;
            }
            hsvToRgb();
            recalculate();
        }
        else if (mRGBMode == RGBMode::RGB_FIRE) {

        }
    }

    return wrapAround;
}
