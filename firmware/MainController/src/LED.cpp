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
        uint8_t intensity = mRGBMode == RGBMode::RGB_DIMMED ? mDimmedIntensity : 255;
        
        FastLED.setBrightness(intensity);
        FastLED.show();
    }

    analogWrite(PIN_LAMP1, 255 - mIntensity[LED_LAMP1]);
    analogWrite(PIN_LAMP2, 255 - mIntensity[LED_LAMP2]);
}

void LEDDriver::updateLEDs(bool force) {
    if (mEnableLEDStrip) {
        switch (mRGBMode) {
            case RGBMode::RGB_ON:
            case RGBMode::RGB_CYCLE:
                CRGB rgb;
                hsv2rgb_rainbow(mHSV, rgb);
                fill_solid(mLEDs, NUM_LEDS, rgb);
                break;
            case RGBMode::RGB_FIRE:

                break;
        }
        FastLED.show();
    }
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
    mEnableLamps = enabled;
    mSettings.setLampEnabled(enabled);
    updateIntensity();
    if (publish) {
        mMqttClient.publish(MQS_LEDS, TOPIC_LAMPS_ENABLE, strBool(mEnableLamps), true);
    }
}

void LEDDriver::enableLEDStrip(bool enabled, bool publish)
{
    mEnableLEDStrip = enabled;
    mSettings.setLEDStripEnabled(enabled);

    digitalWrite(PIN_RGB_PWR, mEnableLEDStrip ? HIGH : LOW);

    updateIntensity();
    updateLEDs(true);

    if (publish) {
        mMqttClient.publish(MQS_LEDS, TOPIC_LEDS_ENABLE, strBool(mEnableLEDStrip), true);
    }
}

void LEDDriver::setRGBMode(RGBMode mode, bool publish)
{
    mRGBMode = mode;
    mSettings.setRGBMode(mode);
    updateIntensity();
    updateLEDs(true);
    if (publish) {
        mMqttClient.publish(MQS_LEDS, TOPIC_RGBMODE, strRGBMode(mRGBMode), true);
    }
}

void LEDDriver::setIntensity(uint8_t value, bool publish)
{
    mLightIntensity = value;
    mSettings.setIntensity(value);
    updateIntensity();

    if (publish) {
        String sIntensity(mLightIntensity);
        mMqttClient.publish(MQS_LEDS, TOPIC_INTENSITY, sIntensity.c_str(), true);
    }
}

void LEDDriver::setDimmedIntensity(uint8_t value, bool publish)
{
    mDimmedIntensity = value;
    mSettings.setDimmedIntensity(value);
    updateIntensity();

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
    mHSV.setHSV(hue, saturation, value);
    mSettings.setHSV(hue, saturation, value);
    updateLEDs(true);
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

    auto callback = std::bind(&LEDDriver::mqttCallback, this, _1, _2, _3);
    auto subscribeCallback = std::bind(&LEDDriver::subscribeCallback, this);

    mMqttClient.registerClient(MQS_LEDS, callback, subscribeCallback);
}

void LEDDriver::loop()
{
    EVERY_N_MILLISECONDS( 20 ) {
        if (mRGBMode == RGBMode::RGB_CYCLE) {
            // 8-bit component, intentionally overflow to cycle around
            mHSV[0]++;
            updateLEDs();
        }
        else if (mRGBMode == RGBMode::RGB_FIRE) {

        }
    }
}
