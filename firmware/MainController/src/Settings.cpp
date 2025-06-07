/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Implementation of EEPROM access.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "Settings.h"

#include <Preferences.h>

#include <commands.h>

static const char* PREF_NAMESPACE = "Prefs";

Preferences myPrefs;

Settings::Settings()
{
}

void Settings::begin()
{
    myPrefs.begin(PREF_NAMESPACE);
}

uint8_t Settings::lightMode()
{
    return myPrefs.getUChar("lightMode", 0);
}

void Settings::setLightMode(uint8_t mode)
{
    myPrefs.putUChar("lightMode", mode);
}

uint8_t Settings::rgbMode()
{
    return myPrefs.getUChar("rgbMode", RGBMode::RGB_ON);
}

void Settings::setRGBMode(uint8_t mode)
{
    myPrefs.putUChar("rgbMode", mode);
}

uint8_t Settings::intensity()
{
    return myPrefs.getUChar("intensity", 255);
}

void Settings::setIntensity(uint8_t value)
{
    myPrefs.putUChar("intensity", value);
}

uint8_t Settings::dimmedIntensity()
{
    return myPrefs.getUChar("dimIntensity", 40);
}

void Settings::setDimmedIntensity(uint8_t value)
{
    myPrefs.putUChar("dimIntensity", value);
}

uint8_t Settings::getHSV(int index)
{
    uint8_t hsv[3];
    myPrefs.getBytes("hsv", hsv, sizeof(hsv));
    return index < 3 ? hsv[index] : 0;
}

void Settings::setHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
    uint8_t hsv[3] = {hue, saturation, value};
    myPrefs.putBytes("hsv", hsv, sizeof(hsv));
}
