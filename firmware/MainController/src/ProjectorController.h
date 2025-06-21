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

using StatusCallback = void(*)(bool switchState, bool powerOn);

static const int COM_BUF_SIZE = 4;

class ProjectorController {
    private:
        StatusCallback mStatusCallback = nullptr;

        ProjectorCommand mMode = PROJECTOR_OFF;

        uint8_t mCntPulse = 0;

        Settings mSettings;

        int     mBufferSize = 0;
        uint8_t mCommandData[COM_BUF_SIZE];

        void processSerialData(uint8_t data);

    public:
        explicit ProjectorController(Settings &settings);
        
        void setStatusCallback(StatusCallback callback) { mStatusCallback = callback; }

        void setMode(ProjectorCommand mode);

        ProjectorCommand mode() const { return mMode; }

        void moveProjector(LiftCommand direction);

        void moveScreen(LiftCommand direction);

        void requestStatus();

        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        void loop();
};
