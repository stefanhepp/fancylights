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

static const uint8_t PIN_SW_TXD       = PIN_PB0;
static const uint8_t PIN_SW_RXD       = PIN_PB7;
static const uint8_t PIN_R            = PIN_PB1;
static const uint8_t PIN_G            = PIN_PB2;
static const uint8_t PIN_B            = PIN_PD3;
static const uint8_t PIN_LAMP1        = PIN_PD5;
static const uint8_t PIN_LAMP2        = PIN_PD6;
static const uint8_t PIN_SCREEN_UP    = PIN_PC1;
static const uint8_t PIN_SCREEN_DOWN  = PIN_PC0;
static const uint8_t PIN_WINCH_SWITCH = PIN_PB6;
static const uint8_t PIN_MOTOR_IN1    = PIN_PC2;
static const uint8_t PIN_MOTOR_IN2    = PIN_PC3;
