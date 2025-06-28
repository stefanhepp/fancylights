/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Store config values in EEPROM.
 * Channel and intensity are values from 0 to 15.
 *
 * Copyright 2025 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <Arduino.h>

#include <inttypes.h>

#include <chsv.h>

#include <commands.h>

class Settings
{
    public:
        Settings();

        bool    isLampEnabled();

        bool    isLEDStripEnabled();

        RGBMode rgbMode();

        uint8_t intensity();

        uint8_t dimmedIntensity();

        CHSV getHSV();

        String getWiFiSSID();

        String getWiFiPassword();

        String getHostname();

        String getMQTTServer();

        String getMQTTTopic();

        uint16_t getMQTTPort();


        void setLampEnabled(bool enabled);

        void setLEDStripEnabled(bool enabled);

        void setRGBMode(RGBMode mode);

        void setIntensity(uint8_t intensity);

        void setDimmedIntensity(uint8_t intensity);

        void setHSV(uint8_t hue, uint8_t saturation, uint8_t value);

        void setWiFiAccess(const char *ssid, const char *password);

        void setHostname(const char *hostname);

        void setMQTTServer(const char *server, uint16_t port, const char *topic);

        void begin();
};

#endif
