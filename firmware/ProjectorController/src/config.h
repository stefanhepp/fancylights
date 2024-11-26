/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * common custom IO and config settings.
 *
 * Copyright (c) 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <Arduino.h>

/* ==================================================== */
/* Common controller settings                           */
/* ==================================================== */

static const uint8_t PIN_SW_TXD       = PIN_PD3;
static const uint8_t PIN_SW_RXD       = PIN_PD2;

static const uint8_t PIN_ENDSTOP      = PIN_PB1;
static const uint8_t PIN_SERVO_ENABLE = PIN_PC2;
static const uint8_t PIN_SERVO_PWM    = PIN_PD6;
