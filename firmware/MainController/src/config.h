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

static const uint8_t PIN_RGB_DATA     = 22;
static const uint8_t PIN_RGB_PWR      = 23;
static const uint8_t PIN_LAMP1        = 19;
static const uint8_t PIN_LAMP2        = 21;
static const uint8_t PIN_SCREEN_UP    = 4;
static const uint8_t PIN_SCREEN_DOWN  = 2;
static const uint8_t PIN_WINCH_SWITCH = 27;
static const uint8_t PIN_MOTOR_IN1    = 25;
static const uint8_t PIN_MOTOR_IN2    = 26;

static const uint8_t UART_KEYPAD      = 1;
static const uint8_t UART_PROJECTOR   = 2;

static const uint8_t PIN_KP_TXD       = 5;
static const uint8_t PIN_KP_RXD       = 18;
static const uint8_t PIN_PR_TXD       = 17;
static const uint8_t PIN_PR_RXD       = 16;
