/*
 * @project     Midi Pedalboard
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Implementation of EEPROM access.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "Settings.h"

#include <commands.h>

#include <avr/eeprom.h>

uint8_t EEMEM confColor[3];
uint8_t EEMEM confLightMode;
uint8_t EEMEM confLightIntensity;
uint8_t EEMEM confDimmedIntensity;
uint8_t EEMEM confRGBMode;

Settings::Settings()
{
}

uint8_t Settings::lightMode()
{
    uint8_t value = eeprom_read_byte(&confLightMode);
    return value == 0xFF ? 0 : value;
}

void Settings::setLightMode(uint8_t mode)
{
    eeprom_write_byte(&confLightMode, mode);
}

uint8_t Settings::rgbMode()
{
    uint8_t value = eeprom_read_byte(&confRGBMode);
    return value == 0xFF ? RGB_ON : value;
}

void Settings::setRGBMode(uint8_t mode)
{
    eeprom_write_byte(&confRGBMode, mode);
}

uint8_t Settings::intensity()
{
    return eeprom_read_byte(&confLightIntensity);
}

void Settings::setIntensity(uint8_t value)
{
    eeprom_write_byte(&confLightIntensity, value);
}

uint8_t Settings::dimmedIntensity()
{
    return eeprom_read_byte(&confDimmedIntensity);
}

void Settings::setDimmedIntensity(uint8_t value)
{
    eeprom_write_byte(&confDimmedIntensity, value);
}

uint8_t Settings::getColor(int index)
{
    return eeprom_read_byte(&confColor[index]);
}

void Settings::setColor(int index, uint8_t intensity)
{
    eeprom_write_byte(&confColor[index], intensity);
}
