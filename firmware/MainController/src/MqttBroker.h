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

class MqttBroker
{
    private:
        Settings &mSettings;

        WiFiClient mWifiClient;
        PubSubClient mMqttClient;

        void mqttCallback(char* topic, byte *payload, unsigned int length);

    public:
        MqttBroker(Settings &settings);

        bool connected() { return mMqttClient.connected(); }

        bool connect();

        void setup();

        void loop();
};
