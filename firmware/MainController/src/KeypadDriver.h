/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Keypad interface driver.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <inttypes.h>

#include <commands.h>

#include "Settings.h"
#include "ProjectorController.h"
#include "LED.h"

static const int UART_BUFFER_SIZE = 16;

class KeypadDriver {
    private:
        Settings   &mSettings;
        LEDDriver  &mLEDs;
        ProjectorController &mProjector;

        uint8_t UARTBuffer[UART_BUFFER_SIZE];
        uint8_t UARTBufferLength = 0;

        // Value 0 means no timeout running, 1 means ready to send, >1 means countdown running
        uint8_t CntStatusTimeout = 0;

        bool checkStatusTimeout();

        void processKeypadCommand(uint8_t data);
    public:
        explicit KeypadDriver(Settings &settings, LEDDriver &leds, ProjectorController &projector);

        void sendKeypadStatus();

        /**
         * Start a timeout to send a status reply to the keypad
         */
        void startStatusTimeout();

        /**
         * Indicate that the status is ready to be sent to the keypad
         */
        void sendStatus();
        /**
         * Initialize all input ports and routines.
         **/
        void begin();

        /**
         * Update LED PWM status.
         */
        void loop();
};

