/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * MQTT client class.
 *
 * Copyright 2025 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

#include <commands.h>

#include "Settings.h"

using MqttTopicCallback = std::function<void(const char *key, byte *payload, unsigned int length)>;
using MqttSubscribeCallback = std::function<void()>;

const char *strRGBMode(RGBMode mode);

const char *strProjectorCommand(ProjectorCommand cmd);

const char *strLiftCommand(LiftCommand cmd);

const char *strBool(bool value);

bool parseRGBMode(const char *str, RGBMode &mode);

bool parseProjectorCommand(const char *str, ProjectorCommand &cmd);

bool parseLiftCommand(const char *str, LiftCommand &cmd);

bool parseBool(const char* str, bool &value);

enum MqttSubtopic
{
    MQS_LEDS,
    MQS_PROJECTOR
};

class MqttClient
{
    private:
        Settings &mSettings;

        String   mTopic;

        WiFiClient mWifiClient;
        PubSubClient mMqttClient;

        MqttTopicCallback mLEDCallback = nullptr;
        MqttTopicCallback mProjectorCallback = nullptr;
        MqttSubscribeCallback mLEDSubscribeCallback = nullptr;
        MqttSubscribeCallback mProjectorSubscribeCallback = nullptr;

        void mqttCallback(char* topic, byte *payload, unsigned int length);

    public:
        MqttClient(Settings &settings);

        void registerClient(MqttSubtopic subtopic, MqttTopicCallback callback,
                            MqttSubscribeCallback subscribeCallback);

        void publish(MqttSubtopic subtopic, const char* key, const char* value, bool subscribe = false);

        bool connected() { return mMqttClient.connected(); }

        bool connect();

        void setup();

        void loop();
};
