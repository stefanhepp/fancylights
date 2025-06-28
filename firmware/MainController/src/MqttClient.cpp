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

#include <commands.h>

#include "config.h"

MqttClient::MqttClient(Settings &settings)
: mSettings(settings), mMqttClient(mWifiClient)
{
}

bool MqttClient::connect()
{
    if (mMqttClient.connect(mSettings.getMQTTClientID().c_str(), 
                            mSettings.getMQTTUsername().c_str(),
                            mSettings.getMQTTPassword().c_str()))
    {
        Serial.printf("[MQTT] Client %s connected!\n", mSettings.getMQTTClientID().c_str());
    } else {
        Serial.printf("[MQTT] Connect failed (state %d)\n", mMqttClient.state());
    }
}

void MqttClient::setup(MqttCallback callback)
{
    mMqttClient.setServer(mSettings.getMQTTServer().c_str(), mSettings.getMQTTPort());
    mMqttClient.setCallback(callback);
}

void MqttClient::loop()
{
    mMqttClient.loop();
}
