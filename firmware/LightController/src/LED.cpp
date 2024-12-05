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


#define HSV_HUE_SEXTANT		256
#define HSV_HUE_STEPS		(6 * HSV_HUE_SEXTANT)

#define HSV_HUE_MIN		0
#define HSV_HUE_MAX		(HSV_HUE_STEPS - 1)
#define HSV_SAT_MIN		0
#define HSV_SAT_MAX		255
#define HSV_VAL_MIN		0
#define HSV_VAL_MAX		255

/********************************************************************* 
 * HSV to RGB calculation, using implementation from:
 * https://www.vagrearg.org/content/hsvrgb
 * Copyright (c) 2016  B. Stultiens
 *********************************************************************/
/*
 * Pointer swapping:
 * 	sext.	r g b	r<>b	g<>b	r <> g	result
 *	0 0 0	v u c			        !u v c	u v c
 *	0 0 1	d v c				            d v c
 *	0 1 0	c v u	u v c			        u v c
 *	0 1 1	c d v	v d c		     d v c	d v c
 *	1 0 0	u c v		     u v c		    u v c
 *	1 0 1	v c d		     v d c	 d v c	d v c
 *
 * if(sextant & 2)
 * 	r <-> b
 *
 * if(sextant & 4)
 * 	g <-> b
 *
 * if(!(sextant & 6) {
 * 	if(!(sextant & 1))
 * 		r <-> g
 * } else {
 * 	if(sextant & 1)
 * 		r <-> g
 * }
 */
#define HSV_SWAPPTR(a,b)	do { uint8_t *tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define HSV_POINTER_SWAP(sextant,r,g,b) \
	do { \
		if((sextant) & 2) { \
			HSV_SWAPPTR((r), (b)); \
		} \
		if((sextant) & 4) { \
			HSV_SWAPPTR((g), (b)); \
		} \
		if(!((sextant) & 6)) { \
			if(!((sextant) & 1)) { \
				HSV_SWAPPTR((r), (g)); \
			} \
		} else { \
			if((sextant) & 1) { \
				HSV_SWAPPTR((r), (g)); \
			} \
		} \
	} while(0)


void LEDDriver::hsvToRgb()
{
    uint16_t h = mHSV[0] * 6;
    uint8_t  s = mHSV[1];
    uint8_t  v = mHSV[2];

    uint8_t *r = &mRGB[0];
    uint8_t *g = &mRGB[1];
    uint8_t *b = &mRGB[2];

    if (s == 0) {
        mRGB[0] = mRGB[1] = mRGB[2] = v;
    }

    uint8_t sextant = h >> 8;

    HSV_POINTER_SWAP(sextant, r, g, b);     // Swap pointers depending which sextant we are in

	*g = v;		// Top level

	// Perform actual calculations
	uint8_t bb;
	uint16_t ww;

	/*
	 * Bottom level: v * (1.0 - s)
	 * --> (v * (255 - s) + error_corr) / 256
	 */
	bb = ~s;
	ww = v * bb;
	ww += 1;		// Error correction
	ww += ww >> 8;		// Error correction
	*b = ww >> 8;

	uint8_t h_fraction = h & 0xff;	// 0...255

	if(!(sextant & 1)) {
		// *r = ...slope_up...;
		/*
		 * Slope up: v * (1.0 - s * (1.0 - h))
		 * --> (v * (255 - (s * (256 - h) + error_corr1) / 256) + error_corr2) / 256
		 */
		ww = !h_fraction ? ((uint16_t)s << 8) : (s * (uint8_t)(-h_fraction));
		ww += ww >> 8;		// Error correction 1
		bb = ww >> 8;
		bb = ~bb;
		ww = v * bb;
		ww += v >> 1;		// Error correction 2
		*r = ww >> 8;
	} else {
		// *r = ...slope_down...;
		/*
		 * Slope down: v * (1.0 - s * h)
		 * --> (v * (255 - (s * h + error_corr1) / 256) + error_corr2) / 256
		 */
		ww = s * h_fraction;
		ww += ww >> 8;		// Error correction 1
		bb = ww >> 8;
		bb = ~bb;
		ww = v * bb;
		ww += v >> 1;		// Error correction 2
		*r = ww >> 8;
	}
}

void LEDDriver::recalculate()
{
    if (mLightMode & 0x01) {
        uint8_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : mLightIntensity;

        mIntensity[LED_LAMP1] = intensity;
        mIntensity[LED_LAMP2] = intensity;
    } else {
        mIntensity[LED_LAMP1] = 0;
        mIntensity[LED_LAMP2] = 0;
    }

    if (mLightMode & 0x02) {
        uint16_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : 256;

        mIntensity[LED_R] = (((uint16_t)mRGB[LED_R])*intensity)/256;
        mIntensity[LED_G] = (((uint16_t)mRGB[LED_G])*intensity)/256;
        mIntensity[LED_B] = (((uint16_t)mRGB[LED_B])*intensity)/256;
    } else {
        mIntensity[LED_R] = 0;
        mIntensity[LED_G] = 0;
        mIntensity[LED_B] = 0;
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

void LEDDriver::setHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
    mHSV[0] = hue;
    mHSV[1] = saturation;
    mHSV[2] = value;
    mSettings.setHSV(0, hue);
    mSettings.setHSV(1, saturation);
    mSettings.setHSV(2, value);
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
        mHSV[i] = mSettings.getHSV(i);
    }
    hsvToRgb();

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
        mCycleCounter = (mCycleCounter + 1) % 8;

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
