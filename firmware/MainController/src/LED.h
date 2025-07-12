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

#include <FastLED.h>
#include <chsv.h>
#include <crgb.h>

#include <commands.h>

#include "Settings.h"
#include "MqttClient.h"

static const uint8_t NUM_LAMPS = 2;

static const uint8_t LED_LAMP1 = 0;
static const uint8_t LED_LAMP2 = 1;

static const uint8_t NUM_LEDS = 108*2+8;

static const uint8_t NUM_LEDS_CENTER = 8;

class LEDDriver {
    private:
        enum LEDAnimation {
            // No next animation scheduled
            ANIM_NONE,
            // Turn power off
            ANIM_DISABLED,
            // All LEDS same color, no animation
            ANIM_ON,
            // All LEDs same color, cycle color
            ANIM_COLORCYCLE,
            // Transition: Fade in; param = size of bar
            ANIM_FADE_IN,
            // Transition: Fade out; param = size of bar
            ANIM_FADE_OUT,
            // Show a bar; param = size of bar (number of leds)
            ANIM_BAR,
            // Show a single dot; param = position (pixel number)
            ANIM_CIRCLE,
            // Show a dot on both sides; param = position (pixel)
            ANIM_SCAN,
            // Fire animation; param = frame
            ANIM_FIRE,
            // Juggle animation
            ANIM_JUGGLE,
            // BPM animation
            ANIM_BPM,
            // rainbow animation
            ANIM_RAINBOW,
            // water animation
            ANIM_WATER
        };

        enum LEDEffect {
            // fill all with same color
            EF_FILLED,
            // show a bar; param = size of bar
            EF_BAR,
            // show a single dot; param = position
            EF_DOT,
            // Show fire animation
            EF_FIRE,
            // Juggle animation; param = number of dots
            EF_JUGGLE,
            // BPM animation; param = 
            EF_BPM,
            // fill rainbow 
            EF_RAINBOW,
            // water animation
            EF_WATER
        };

        Settings   &mSettings;
        MqttClient &mMqttClient;

        CHSV    mHSV;
    
        uint8_t mIntensity[NUM_LAMPS];
        CRGB    mLEDs[NUM_LEDS];

        bool    mEnableLamps = false;
        bool    mEnableLEDStrip = false;

        uint8_t mLightIntensity = 255;
        uint8_t mDimmedIntensity = 50;

        RGBMode mRGBMode = RGB_ON;

        LEDAnimation  mAnimation = ANIM_NONE;
        LEDAnimation  mNextAnimation = ANIM_NONE;
        bool          mAnimationDirection;

        LEDEffect     mEffect;
        int           mEffectParam;
        int           mEffectCount;
        // Mirror the effect on both sides, otherwise use the full length
        bool          mEffectMirrored = false;
        CRGBPalette16 mEffectPalette;
        // BPM value for some effects
        int           mEffectBPM = 13;
        // Speed to fade out old pixels
        int           mFadeSpeed = 20;
        // Add some glitter effect; 0 = off, 255: full
        int           mGlitterChance = 0;

        void updateLamps();

        void updateLEDs();

        void updateAnimation();

        LEDAnimation getAnimation(RGBMode mode);

        void setNextAnimation(LEDAnimation animation);

        void startAnimation(LEDAnimation animation);

        void startFading(bool fadeOut, LEDAnimation nextAnimation);

        bool isFading() const { return mAnimation == ANIM_FADE_IN || mAnimation == ANIM_FADE_OUT; }

        bool isAnimationFinished() const;

        void mqttCallback(const char *key, const char* payload, unsigned int length);

        void subscribeCallback();

        void publishColor(bool subscribe = false);

        void appendHexCode(String &rgb, uint8_t val);

    public:
        explicit LEDDriver(Settings &settings, MqttClient &mqttClient);


        bool    isLampEnabled() { return mEnableLamps; }

        bool    isLEDStripEnabled() { return mEnableLEDStrip; }

        uint8_t intensity() const { return mLightIntensity; }

        uint8_t dimmedIntensity() const { return mDimmedIntensity; }


        RGBMode rgbMode() const { return mRGBMode; }

        const CHSV &getHSV() const { return mHSV; }


        void enableLamps(bool enabled, bool publish = true);

        void enableLEDStrip(bool enabled, bool publish = true);

        void setIntensity(uint8_t intensity, bool publish = true);

        void setDimmedIntensity(uint8_t intensity, bool publish = true);


        void setRGBMode(RGBMode mode, bool publish = true);

        void setHSV(CHSV hsv, bool publish = true);

        void setHSV(uint8_t hue, uint8_t saturation, uint8_t value, bool publish = true);

        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        /**
         * Update LED PWM status.
         */
        void loop();
};
