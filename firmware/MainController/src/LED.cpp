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

//#define FASTLED_ESP32_I2S

#include "LED.h"

#include <functional>
#include <ArduinoJson.h>
#include <stdio.h>

#include <commands.h>

#include <FastLED.h>

#include "config.h"

const char *TOPIC_LAMPS_ENABLE = "lamps";
const char *TOPIC_LEDS_ENABLE = "leds";
const char *TOPIC_INTENSITY = "intensity";
const char *TOPIC_DIMMED_INTENSITY = "dimmed";
const char *TOPIC_RGBMODE = "mode";
const char *TOPIC_COLOR_HSV = "hsv";
const char *TOPIC_COLOR_RGB = "rgb";


LEDDriver::LEDDriver(Settings &settings, MqttClient &mqttClient)
: mSettings(settings), mMqttClient(mqttClient)
{
    for (uint8_t i = 0; i < NUM_LAMPS; i++) {
        mIntensity[i] = 0;
    }
}

void LEDDriver::updateLamps()
{
    if (mEnableLamps) {
        uint8_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : mLightIntensity;

        mIntensity[LED_LAMP1] = intensity;
        mIntensity[LED_LAMP2] = intensity;
    } else {
        mIntensity[LED_LAMP1] = 0;
        mIntensity[LED_LAMP2] = 0;
    }

    analogWrite(PIN_LAMP1, 255 - mIntensity[LED_LAMP1]);
    analogWrite(PIN_LAMP2, 255 - mIntensity[LED_LAMP2]);
}

void LEDDriver::updateLEDs() {
    CHSV fullColor = mHSV;
    CRGB rgb;

    CHSV hsv2;
    CRGB rgb2;

    int i;

    fullColor.value = 255;
    hsv2rgb_rainbow(fullColor, rgb);

    if (mAnimation != ANIM_NONE) {
        switch (mEffect) {
            case EF_FILLED:
                fill_solid(mLEDs, NUM_LEDS, rgb);
                break;
            case EF_BAR:
                // get half-bright pixel color
                hsv2 = mHSV;
                hsv2.v = mHSV.v/2;
                hsv2rgb_rainbow(hsv2, rgb2);

                for (int i = 0; i < NUM_LEDS; i++) {
                    CRGB color;
                    if (i < mEffectParam || (mEffectMirrored && i > NUM_LEDS - 1 - mEffectParam)) {
                        color = rgb;
                    }
                    else if (i == mEffectParam || (mEffectMirrored && i == NUM_LEDS - 1 - mEffectParam)) {
                        color = rgb2;
                    }
                    else {
                        color = CRGB::Black;
                    }
                    mLEDs[i] = color;
                }
                break;
            case EF_DOT:
                fadeToBlackBy(mLEDs, NUM_LEDS, mFadeSpeed);
                for (i = 0; i < mEffectCount; i++) {
                    int idx = mEffectParam + i*(NUM_LEDS/mEffectCount);
                    mLEDs[idx] = mHSV;
                    if (mEffectMirrored) {
                        mLEDs[NUM_LEDS - 1 - idx] = mHSV;
                    }
                }
                break;
            case EF_FIRE:
                break;
            case EF_JUGGLE:
                // colored dots, weaving in and out of sync with each other
                fadeToBlackBy(mLEDs, NUM_LEDS, mFadeSpeed);
                for (i = 0; i < mEffectCount; i++) {
                    rgb2 = ColorFromPalette(mEffectPalette, mHSV.hue+(i*127)/mEffectCount, 255);
                    int idx = beatsin16(i+mEffectParam, 0, mEffectMirrored ? NUM_LEDS/2 : NUM_LEDS-1);
                    mLEDs[idx] |= rgb2;
                    if (mEffectMirrored) {
                        mLEDs[NUM_LEDS - 1 - idx] |= rgb2;
                    }
                }
                break;
            case EF_BPM:
                fadeToBlackBy(mLEDs, NUM_LEDS, mFadeSpeed);
                for (i = 0; i < NUM_LEDS-1; i++) {
                    mLEDs[i] = ColorFromPalette(mEffectPalette, mHSV.hue + (i*2), mEffectParam - mHSV.hue + (i*10));
                }
                break;
            case EF_RAINBOW:
                fill_rainbow(mLEDs, NUM_LEDS, mHSV.hue, 7);
                break;
            case EF_WATER:
                
                break;
        }

        if (mGlitterChance > 0) {
            if ( random8() < mGlitterChance ) {
                mLEDs[ random16(NUM_LEDS) ] += CRGB::White;
            }
        }

        if (mRGBMode == RGB_DIMMED) {
            FastLED.setBrightness((((int)mHSV.value) * mDimmedIntensity)/ 255);
        } else {
            FastLED.setBrightness(mHSV.value);
        }
        FastLED.show();
    }
}

void LEDDriver::updateAnimation()
{
    if (isAnimationFinished() && mNextAnimation != ANIM_NONE) {
        startAnimation(mNextAnimation);        
        mNextAnimation = ANIM_NONE;
    }

    switch (mAnimation) {
        case ANIM_NONE:
        case ANIM_ON:
        case ANIM_DISABLED:
            break;
        case ANIM_JUGGLE:
        case ANIM_COLORCYCLE:
        case ANIM_RAINBOW:
            // Color cycle, wrap around at 255
            mHSV[0]++;
            break;
        case ANIM_FADE_IN:
            mEffectParam += 3;
            break;
        case ANIM_FADE_OUT:
            mEffectParam -= 4;
            break;
        case ANIM_CIRCLE:
            mEffectParam = (mEffectParam + 1) % (NUM_LEDS / 2);
            break;
        case ANIM_SCAN:
            mEffectParam = beatsin16(mEffectBPM, 0, (NUM_LEDS-NUM_LEDS_CENTER)/2 - 1);
            break;
        case ANIM_FIRE:
        case ANIM_WATER:
        case ANIM_BPM:
            mHSV[0]++;
            mEffectParam = beatsin8(mEffectBPM, 64, 255);
            break;
    }

    if (mAnimation != ANIM_NONE) {
        updateLEDs();
    }
}

void LEDDriver::startAnimation(LEDAnimation animation)
{
    if (mAnimation == animation) {
        // Dont reset current animation if already started
        return;
    }

    mAnimation = animation;

    mEffectCount = 1;
    mEffectMirrored = false;
    mGlitterChance = 0;

    // Set the effect for the animation
    switch (mAnimation) {
        case ANIM_DISABLED:
            // Disable power and turn off
            digitalWrite(PIN_RGB_PWR, LOW);
            mAnimation = ANIM_NONE;
            // fallthrough..
        case ANIM_NONE:
        case ANIM_ON:
        case ANIM_COLORCYCLE:
            mEffect = EF_FILLED;
            break;
        case ANIM_FADE_IN:
            mEffect = EF_BAR;
            if (!isFading()) {
                mEffectParam = 0;
            }
            mEffectMirrored = true;
            break;
        case ANIM_FADE_OUT:
            mEffect = EF_BAR;
            if (!isFading()) {
                mEffectParam = NUM_LEDS / 2;
            }
            mEffectMirrored = true;
            break;
        case ANIM_CIRCLE:
            mEffect = EF_DOT;
            mEffectCount = 2;
            mEffectMirrored = false;
            break;
        case ANIM_SCAN:
            mEffect = EF_DOT;
            mEffectCount = 1;
            mEffectMirrored = true;
            break;
        case ANIM_FIRE:
            mEffect = EF_BPM;
            mEffectPalette = HeatColors_p;
            mGlitterChance = 100;
            break;
        case ANIM_JUGGLE:
            mEffect = EF_JUGGLE;
            mEffectPalette = PartyColors_p;
            mEffectCount = 5;
            mEffectParam = 6;
            mEffectMirrored = false;
            break;
        case ANIM_BPM:
            mEffect = EF_BPM;
            mEffectPalette = PartyColors_p;
            break;
        case ANIM_RAINBOW:
            mEffect = EF_RAINBOW;
            mGlitterChance = 80;
            break;
        case ANIM_WATER:
            mEffect = EF_BPM;
            mEffectPalette = OceanColors_p;
            mGlitterChance = 30;
            break;
    }
}

void LEDDriver::startFading(bool fadeOut, LEDAnimation nextAnimation)
{
    startAnimation( fadeOut ? ANIM_FADE_OUT : ANIM_FADE_IN );
    setNextAnimation( nextAnimation );
}

bool LEDDriver::isAnimationFinished() const
{
    if (mAnimation == ANIM_FADE_IN && mEffectParam < NUM_LEDS / 2) {
        return false;
    } 
    if (mAnimation == ANIM_FADE_OUT && mEffectParam > 0) {
        return false;
    }
    // All other animations immediately terminate
    return true;
}

LEDDriver::LEDAnimation LEDDriver::getAnimation(RGBMode mode)
{
    switch (mode) {
        case RGB_ON:
        case RGB_DIMMED:
            return ANIM_ON;
        case RGB_CYCLE:
            return ANIM_COLORCYCLE;
        case RGB_FIRE:
            return ANIM_FIRE;
        case RGB_SPIN:
            return ANIM_CIRCLE;
        case RGB_SCAN:
            return ANIM_SCAN;
        case RGB_JUGGLE:
            return ANIM_JUGGLE;
        case RGB_BPM:
            return ANIM_BPM;
        case RGB_RAINBOW:
            return ANIM_RAINBOW;
        case RGB_WATER:
            return ANIM_WATER;
    }
    return ANIM_NONE;
}

void LEDDriver::setNextAnimation(LEDAnimation animation)
{
    mNextAnimation = animation;
}

void LEDDriver::mqttCallback(const char *key, const char* payload, unsigned int length)
{
    if (length == 0) {
        return;
    }
    if (strcmp(key, TOPIC_LAMPS_ENABLE) == 0) {
        bool val;
        if (parseBool(payload, val)) {
            enableLamps(val, false);
        }
    }
    if (strcmp(key, TOPIC_LEDS_ENABLE) == 0) {
        bool val;
        if (parseBool(payload, val)) {
            enableLEDStrip(val, false);
        }
    }
    if (strcmp(key, TOPIC_INTENSITY) == 0) {
        int val;
        int result = sscanf(payload, "%d", &val);
        if (result == 1 && val >= 0 && val < 256) {
            setIntensity(val, false);
        }
    }
    if (strcmp(key, TOPIC_DIMMED_INTENSITY) == 0) {
        int val;
        int result = sscanf(payload, "%d", &val);
        if (result == 1 && val >= 0 && val < 256) {
            setDimmedIntensity(val, false);
        }
    }
    if (strcmp(key, TOPIC_RGBMODE) == 0) {
        RGBMode mode;
        if (parseRGBMode(payload, mode)) {
            setRGBMode(mode, false);
        }
    }
    if (strcmp(key, TOPIC_COLOR_RGB) == 0) {
        if (payload[0] == '#') {
            String srgb = (char*)payload;
            String sr = srgb.substring(1,3);
            String sg = srgb.substring(3,5);
            String sb = srgb.substring(5,7);

            CRGB rgb;
            ;
            rgb.r = strtol(sr.c_str(), 0, 16);
            rgb.g = strtol(sg.c_str(), 0, 16);
            rgb.b = strtol(sb.c_str(), 0, 16);

            CHSV hsv = rgb2hsv_approximate(rgb);

            setHSV(hsv, false);
        } else {
            int r, g, b;
            int result = sscanf(payload, "rgb(%d, %d, %d)", &r, &g, &b);
            if (result == 3) {
                CRGB rgb;
                rgb.r = r;
                rgb.g = g;
                rgb.b = b;

                CHSV hsv = rgb2hsv_approximate(rgb);

                setHSV(hsv, false);
            }
        }
    }
    if (strcmp(key, TOPIC_COLOR_HSV) == 0) {
        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("[MQTT] Deserialization failed: ");
            Serial.println(error.f_str());
            return;
        }

        CHSV hsv;
        hsv.hue = doc["h"].as<unsigned char>();
        hsv.sat = doc["s"].as<unsigned char>();
        hsv.val = doc["v"].as<unsigned char>();

        setHSV(hsv, false);
    }
}

void LEDDriver::subscribeCallback()
{
    String sIntensity(mLightIntensity);
    String sDimmedIntensity(mDimmedIntensity);

    mMqttClient.publish(MQS_LEDS, TOPIC_LAMPS_ENABLE, strBool(mEnableLamps), true);
    mMqttClient.publish(MQS_LEDS, TOPIC_LEDS_ENABLE, strBool(mEnableLEDStrip), true);
    mMqttClient.publish(MQS_LEDS, TOPIC_INTENSITY, sIntensity.c_str(), true);
    mMqttClient.publish(MQS_LEDS, TOPIC_DIMMED_INTENSITY, sDimmedIntensity.c_str(), true);
    mMqttClient.publish(MQS_LEDS, TOPIC_RGBMODE, strRGBMode(mRGBMode), true);

    publishColor(true);
}

void LEDDriver::appendHexCode(String &rgb, uint8_t val) {
    String v(val, HEX);
    if (v.length() == 1)  {
        rgb += "0";
    }
    rgb += v;
}

void LEDDriver::publishColor(bool subscribe) {
    JsonDocument doc;
    doc["h"] = mHSV.hue;
    doc["s"] = mHSV.saturation;
    doc["v"] = mHSV.value;

    String json;
    serializeJson(doc, json);
    mMqttClient.publish(MQS_LEDS, TOPIC_COLOR_HSV, json.c_str(), subscribe);

    CRGB rgb;
    hsv2rgb_rainbow(mHSV, rgb);

    String sRGB = "#";
    appendHexCode(sRGB, rgb.r);
    appendHexCode(sRGB, rgb.g);
    appendHexCode(sRGB, rgb.b);

    mMqttClient.publish(MQS_LEDS, TOPIC_COLOR_RGB, sRGB.c_str(), subscribe);
}

void LEDDriver::enableLamps(bool enabled, bool publish)
{
    if (mEnableLamps == enabled) {
        return;
    }
    mEnableLamps = enabled;
    mSettings.setLampEnabled(enabled);
    updateLamps();
    if (publish) {
        mMqttClient.publish(MQS_LEDS, TOPIC_LAMPS_ENABLE, strBool(mEnableLamps), true);
    }
}

void LEDDriver::enableLEDStrip(bool enabled, bool publish)
{
    if (mEnableLEDStrip == enabled) {
        return;
    }
    mEnableLEDStrip = enabled;
    mSettings.setLEDStripEnabled(enabled);

    if (enabled) {
        digitalWrite(PIN_RGB_PWR, HIGH);
        startFading(false, getAnimation(mRGBMode));
    } else {
        // Power will be disabled after finish fading
        startFading(true, ANIM_DISABLED);
    }

    updateLamps();

    if (publish) {
        mMqttClient.publish(MQS_LEDS, TOPIC_LEDS_ENABLE, strBool(mEnableLEDStrip), true);
    }
}

void LEDDriver::setRGBMode(RGBMode mode, bool publish)
{
    if (mRGBMode == mode) {
        return;
    }
    mRGBMode = mode;
    mSettings.setRGBMode(mode);
    setNextAnimation( getAnimation(mode) );
    updateLamps();
    
    if (publish) {
        mMqttClient.publish(MQS_LEDS, TOPIC_RGBMODE, strRGBMode(mRGBMode), true);
    }
}

void LEDDriver::setIntensity(uint8_t value, bool publish)
{
    if (mLightIntensity == value) {
        return;
    }
    mLightIntensity = value;
    mSettings.setIntensity(value);
    updateLamps();

    if (publish) {
        String sIntensity(mLightIntensity);
        mMqttClient.publish(MQS_LEDS, TOPIC_INTENSITY, sIntensity.c_str(), true);
    }
}

void LEDDriver::setDimmedIntensity(uint8_t value, bool publish)
{
    if (mDimmedIntensity == value) {
        return;
    }
    mDimmedIntensity = value;
    mSettings.setDimmedIntensity(value);
    updateLamps();

    if (publish) {
        String sDimmedIntensity(mDimmedIntensity);
        mMqttClient.publish(MQS_LEDS, TOPIC_DIMMED_INTENSITY, sDimmedIntensity.c_str(), true);
    }
}

void LEDDriver::setHSV(CHSV hsv, bool publish)
{
    setHSV(hsv.h, hsv.s, hsv.v, publish);
}

void LEDDriver::setHSV(uint8_t hue, uint8_t saturation, uint8_t value, bool publish)
{
    if (mHSV[0] == hue && mHSV[1] == saturation && mHSV[2] == value) {
        return;
    }
    mHSV.setHSV(hue, saturation, value);
    mSettings.setHSV(hue, saturation, value);
    updateLEDs();
    if (publish) {
        publishColor();
    }
}

void LEDDriver::begin()
{
    using namespace std::placeholders;

    analogWriteFrequency(5000);
    analogWriteResolution(8);

    pinMode(PIN_RGB_DATA, OUTPUT);
    pinMode(PIN_RGB_PWR, OUTPUT);

    digitalWrite(PIN_RGB_PWR, LOW);

    FastLED.addLeds<WS2815, PIN_RGB_DATA, GRB>(mLEDs, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setDither(BINARY_DITHER);

    mLightIntensity = mSettings.intensity();
    mDimmedIntensity = mSettings.dimmedIntensity();

    mRGBMode = mSettings.rgbMode();

    mHSV = mSettings.getHSV();

    enableLamps( mSettings.isLampEnabled(), false );
    enableLEDStrip( mSettings.isLEDStripEnabled(), false );

    updateLamps();
    updateLEDs();

    auto callback = std::bind(&LEDDriver::mqttCallback, this, _1, _2, _3);
    auto subscribeCallback = std::bind(&LEDDriver::subscribeCallback, this);

    mMqttClient.registerClient(MQS_LEDS, callback, subscribeCallback);
}

void LEDDriver::loop()
{
    EVERY_N_MILLISECONDS( 20 ) {
        updateAnimation();
    }
}
