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

bool Settings::isLampEnabled()
{
    return myPrefs.getUChar("lampOn", 0);
}

bool Settings::isLEDStripEnabled()
{
    return myPrefs.getUChar("LEDOn", 0);
}

void Settings::setLampEnabled(bool enabled)
{
    myPrefs.putUChar("lampOn", enabled);
}

void Settings::setLEDStripEnabled(bool enabled)
{
    myPrefs.putUChar("LEDOn", enabled);
}


RGBMode Settings::rgbMode()
{
    return (RGBMode) myPrefs.getUChar("rgbMode", RGBMode::RGB_ON);
}

void Settings::setRGBMode(RGBMode mode)
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


CHSV Settings::getHSV()
{
    uint8_t hsv[3];
    CHSV chsv;

    myPrefs.getBytes("hsv", hsv, sizeof(hsv));
    chsv.setHSV(hsv[0], hsv[1], hsv[2]);

    return chsv;
}

void Settings::setHSV(uint8_t hue, uint8_t saturation, uint8_t value)
{
    uint8_t hsv[3] = {hue, saturation, value};
    myPrefs.putBytes("hsv", hsv, sizeof(hsv));
}
