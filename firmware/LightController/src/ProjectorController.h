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

class ProjectorController {
    private:
        uint8_t mMode = PROJECTOR_OFF;

        Settings mSettings;

    public:
        explicit ProjectorController(Settings &settings);
        
        void setMode(uint8_t mode);

        uint8_t mode() const { return mMode; }

        void moveProjector(uint8_t direction);

        void moveScreen(uint8_t direction);

        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        void loop();
};
