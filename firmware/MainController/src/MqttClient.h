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

#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>

#include "Settings.h"

using MqttCallback = void(*)(char *topic, byte *payload, unsigned int length);

class MqttClient
{
    private:
        Settings &mSettings;

        WiFiClient mWifiClient;
        PubSubClient mMqttClient;

    public:
        MqttClient(Settings &settings);

        bool connected() { return mMqttClient.connected(); }

        bool connect();

        void setup(MqttCallback callback);

        void loop();
};
