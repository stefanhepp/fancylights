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

using StatusCallback = void(*)(bool powerOn);

class ProjectorController {
    private:
        StatusCallback mStatusCallback = nullptr;

        ProjectorCommand mMode = PROJECTOR_OFF;

        uint8_t mCntPulse = 0;

        Settings mSettings;

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
