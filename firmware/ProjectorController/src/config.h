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

#define PORT_OUT      B
#define PORT_IN       D

/** 
 * Set PISTON_TECHNICS to compile for Technics keyboard pistons,
 * or unset to compile for Keyboard controller pistons.
 */
//#define PISTON_TECHNICS

static const uint8_t PIN_INTERRUPT   = PIN_PC3;

static const uint8_t PIN_ADDR1       = PIN_PC2;
static const uint8_t PIN_ADDR2       = PIN_PC1;
static const uint8_t PIN_ADDR3       = PIN_PC0;

