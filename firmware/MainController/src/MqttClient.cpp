/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * MQTT Client implementation.
 *
 * Copyright 2025 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "MqttClient.h"

#include <functional>

#include <commands.h>

#include "config.h"

const char *TOPIC_LEDS = "leds/";
const char *TOPIC_PROJECTOR = "projector/";

const char *strRGBMode(RGBMode mode)
{
    switch (mode) {
        case RGB_ON:
            return "on";
            break;
        case RGB_CYCLE:
            return "cycle";
            break;
        case RGB_FIRE:
            return "fire";
            break;
        case RGB_DIMMED:
            return "dimmed";
            break;
    }
    return "";
}

const char *strProjectorCommand(ProjectorCommand cmd)
{
    switch (cmd) {
        case PROJECTOR_OFF:
            return "off";
            break;
        case PROJECTOR_ON:
            return "on";
            break;
        case PROJECTOR_NORMAL:
            return "normal";
            break;
        case PROJECTOR_3D:
            return "3D";
            break;
        case PROJECTOR_VR:
            return "VR";
            break;
    }
    return "";
}

const char *strLiftCommand(LiftCommand cmd)
{
    switch (cmd) {
        case LIFT_UP:
            return "up";
            break;
        case LIFT_DOWN:
            return "down";
            break;
        case LIFT_STOP:
            return "stop";
            break;
    }
    return "";
}

const char *strBool(bool value)
{
    return value ? "on" : "off";
}

bool parseRGBMode(const char *str, RGBMode &mode)
{
    if (strcmp(str, "on") == 0) {
        mode = RGB_ON;
        return true;
    }
    if (strcmp(str, "cycle") == 0) {
        mode = RGB_CYCLE;
        return true;
    }
    if (strcmp(str, "fire") == 0) {
        mode = RGB_FIRE;
        return true;
    }
    if (strcmp(str, "dimmed") == 0) {
        mode = RGB_DIMMED;
        return true;
    }
    return false;
}

bool parseProjectorCommand(const char *str, ProjectorCommand &cmd)
{
    if (strcmp(str, "off") == 0) {
        cmd = PROJECTOR_OFF;
        return true;
    }
    if (strcmp(str, "on") == 0) {
        cmd = PROJECTOR_ON;
        return true;
    }
    if (strcmp(str, "normal") == 0) {
        cmd = PROJECTOR_NORMAL;
        return true;
    }
    if (strcmp(str, "3D") == 0) {
        cmd = PROJECTOR_3D;
        return true;
    }
    if (strcmp(str, "VR") == 0) {
        cmd = PROJECTOR_VR;
        return true;
    }
    return false;
}

bool parseLiftCommand(const char *str, LiftCommand &cmd)
{
    if (strcmp(str, "up") == 0) {
        cmd = LIFT_UP;
        return true;
    }
    if (strcmp(str, "down") == 0) {
        cmd = LIFT_DOWN;
        return true;
    }
    if (strcmp(str, "stop") == 0) {
        cmd = LIFT_STOP;
        return true;
    }
    return false;
}

bool parseBool(const char* str, bool &value)
{
    String s = str;
    s.toLowerCase();

    if (s == "on" || s == "true" || s == "1") {
        value = true;
        return true;
    }
    if (s == "off" || s == "false" || s == "0") {
        value = false;
        return true;
    }
    return false;
}

MqttClient::MqttClient(Settings &settings)
: mSettings(settings), mMqttClient(mWifiClient)
{
}

void MqttClient::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("[MQTT] Received topic %s, message: ", topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();

    String stopic(topic);
    String spayload(payload, length);

    if (stopic.startsWith(mTopic)) {
        String subtopic = stopic.substring(mTopic.length());
        String key;

        if (subtopic.startsWith(TOPIC_LEDS)) {
            String key = subtopic.substring(strlen(TOPIC_LEDS));
            if (mLEDCallback) {
                mLEDCallback(key.c_str(), spayload.c_str(), length);
            }
        }
        if (subtopic.startsWith(TOPIC_PROJECTOR)) {
            String key = subtopic.substring(strlen(TOPIC_PROJECTOR));
            if (mProjectorCallback) {
                mProjectorCallback(key.c_str(), spayload.c_str(), length);
            }
        }
    }
}

void MqttClient::registerClient(MqttSubtopic subtopic, MqttTopicCallback callback, MqttSubscribeCallback subscribeCallback)
{
    switch (subtopic) {
        case MQS_LEDS:
            mLEDCallback = callback;
            mLEDSubscribeCallback = subscribeCallback;
            if (connected()) {
                mLEDSubscribeCallback();
            }
            break;
        case MQS_PROJECTOR:
            mProjectorCallback = callback;
            mProjectorSubscribeCallback = subscribeCallback;
            if (connected()) {
                mProjectorSubscribeCallback();
            }
            break;
    }
}

void MqttClient::publish(MqttSubtopic subtopic, const char* key, const char* value, bool subscribe)
{
    String topic = mTopic;
    switch (subtopic) {
        case MQS_LEDS:
            topic += TOPIC_LEDS;
            break;
        case MQS_PROJECTOR:
            topic += TOPIC_PROJECTOR;
            break;
    }
    topic += key;

    Serial.printf("[MQTT] Publish %s: %s\n", topic.c_str(), value);

    mMqttClient.publish(topic.c_str(), value, true);
    if (subscribe) {
        mMqttClient.subscribe(topic.c_str());
    }
}

void MqttClient::subscribe(MqttSubtopic subtopic, const char* key)
{
    String topic = mTopic;
    switch (subtopic) {
        case MQS_LEDS:
            topic += TOPIC_LEDS;
            break;
        case MQS_PROJECTOR:
            topic += TOPIC_PROJECTOR;
            break;
    }
    topic += key;

    Serial.printf("[MQTT] Subscribe %s\n", topic.c_str());
    
    mMqttClient.subscribe(topic.c_str());
}

bool MqttClient::connect()
{
    mTopic = mSettings.getMQTTTopic();
    mTopic += "/";

    mMqttClient.disconnect();

    IPAddress mqttAddr;
    WiFi.hostByName(mSettings.getMQTTServer().c_str(), mqttAddr);

    mMqttClient.setServer(mqttAddr, mSettings.getMQTTPort());

    Serial.printf("[MQTT] Connecting to %s:%d/%s (IP: %s) ..\n", 
        mSettings.getMQTTServer().c_str(), mSettings.getMQTTPort(), mTopic,
        mqttAddr.toString().c_str()
    );
    
    if (mMqttClient.connect(mSettings.getMQTTClientID().c_str(), 
                            mSettings.getMQTTUsername().c_str(),
                            mSettings.getMQTTPassword().c_str()))
    {
        Serial.printf("[MQTT] Client %s connected!\n", mSettings.getMQTTClientID().c_str());

        if (mLEDSubscribeCallback) {
            mLEDSubscribeCallback();
        }
        if (mProjectorSubscribeCallback) {
            mProjectorSubscribeCallback();
        }
        return true;
    } else {
        Serial.printf("[MQTT] Connect failed (state %d)\n", mMqttClient.state());
        return false;
    }
}

void MqttClient::setup()
{
    using namespace std::placeholders;

    mMqttClient.setServer(mSettings.getMQTTServer().c_str(), mSettings.getMQTTPort());
    auto callback = std::bind(&MqttClient::mqttCallback, this, _1, _2, _3);

    mMqttClient.setCallback(callback);
}

void MqttClient::loop()
{
    mMqttClient.loop();
}
