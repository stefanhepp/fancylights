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

#include "config.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <avrlib.h>

LEDDriver::LEDDriver(Settings &settings)
: mSettings(settings), mPWMCounter(0), mLastCounter(0)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        mIntensity[i] = 0;
    }
}

void LEDDriver::setLightMode(uint8_t mode)
{
    mLightMode = mode;
    mSettings.setLightMode(mode);
}

void LEDDriver::setRGBMode(uint8_t mode)
{
    mRGBMode = mode;
    mSettings.setRGBMode(mode);
}

void LEDDriver::setIntensity(uint8_t value)
{
    mLightIntensity = value;
    mSettings.setIntensity(value);
}

void LEDDriver::setLEDIntensity(int index, uint8_t intensity)
{
    mIntensity[index] = intensity;
    mSettings.setLEDIntensity(index, intensity);
}

void LEDDriver::begin()
{
    mLightMode = mSettings.lightMode();
    mLightIntensity = mSettings.intensity();
    mRGBMode = mSettings.rgbMode();
    for (int i = 0; i < NUM_LEDS; i++) {
        mIntensity[i] = mSettings.getLEDIntensity(i);
    }

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
    return mPWMCounter < mLastCounter;
}
