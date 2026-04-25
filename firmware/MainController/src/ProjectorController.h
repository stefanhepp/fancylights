/*
 * @project     FancyController
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

#include <commands.h>

#include "Settings.h"
#include "MqttClient.h"

using StatusCallback = void(*)(bool switchState, bool powerOn, bool hasPowerStatus, bool isPowering);
using ModeChangeCallback = void(*)(ProjectorCommand mode);

static const int COM_BUF_SIZE = 4;

class ProjectorController {
    private:
        StatusCallback     mStatusCallback = nullptr;
        ModeChangeCallback mModeChangeCallback = nullptr;

        ProjectorCommand mMode = PROJECTOR_OFF;

        uint8_t mCntPulse = 0;

        unsigned long mPowerChangeTime = 0;

        bool mDelayedStatusRequest = false;

        Settings   &mSettings;
        MqttClient &mMqttClient;

        int     mBufferSize = 0;
        uint8_t mCommandData[COM_BUF_SIZE];

        bool isPowerChanging();

        void processSerialData(uint8_t data);

        void mqttCallback(const char *key, const char* payload, unsigned int length);

        void subscribeCallback();

    public:
        explicit ProjectorController(Settings &settings, MqttClient &mqttClient);
        
        void setStatusCallback(StatusCallback callback) { mStatusCallback = callback; }

        void setModeChangeCallback(ModeChangeCallback callback) { mModeChangeCallback = callback; }

        ProjectorCommand mode() const { return mMode; }

        void sendMode(ProjectorCommand mode, bool publish = true);

        void moveProjector(LiftCommand direction);

        void moveScreen(LiftCommand direction);

        void requestStatus();

        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        void loop();
};
