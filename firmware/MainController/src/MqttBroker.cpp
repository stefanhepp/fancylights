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
#include "MqttBroker.h"

#include <functional>

#include <commands.h>

#include "config.h"

MqttBroker::MqttBroker(Settings &settings)
: mSettings(settings), mMqttClient(mWifiClient)
{
}

void MqttBroker::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("[MQTT] Received topic %s, message: ", topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();



}

bool MqttBroker::connect()
{
    if (mMqttClient.connect(mSettings.getMQTTClientID().c_str(), 
                            mSettings.getMQTTUsername().c_str(),
                            mSettings.getMQTTPassword().c_str()))
    {
        Serial.printf("[MQTT] Client %s connected!\n", mSettings.getMQTTClientID().c_str());
        return true;
    } else {
        Serial.printf("[MQTT] Connect failed (state %d)\n", mMqttClient.state());
        return false;
    }
}

void MqttBroker::setup()
{
    using namespace std::placeholders;

    mMqttClient.setServer(mSettings.getMQTTServer().c_str(), mSettings.getMQTTPort());
    auto callback = std::bind(&MqttBroker::mqttCallback, this, _1, _2, _3);

    mMqttClient.setCallback(callback);
}

void MqttBroker::loop()
{
    mMqttClient.loop();
}
