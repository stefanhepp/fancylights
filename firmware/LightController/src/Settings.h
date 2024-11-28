/*
 * @project     Midi Pedalboard
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Store config values in EEPROM.
 * Channel and intensity are values from 0 to 15.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <inttypes.h>

class Settings
{
    public:
        Settings();

        uint8_t lightMode();

        uint8_t rgbMode();

        uint8_t intensity();

        uint8_t dimmedIntensity();

        uint8_t getColor(int index);


        void setLightMode(uint8_t mode);

        void setRGBMode(uint8_t mode);

        void setIntensity(uint8_t intensity);

        void setDimmedIntensity(uint8_t intensity);

        void setColor(int index, uint8_t intensity);
};

#endif
