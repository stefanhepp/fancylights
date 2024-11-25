/**
 * Project:   FancyLights
 * File:      command.h
 * Author:    Stefan Hepp <stefan@stefant.org>
 *
 * Copyright 2021 Stefan Hepp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Common UART and I2C command codes definitions for all boards.
 *
 * For UART messages, we use the following message structure:
 * <CMD HEADER><CMD OPCODE> | <VALUE1> | [<VALUE2>] | ...
 *
 */
#pragma once

#include <stdint.h>

static const uint8_t KEYPAD_I2C_ADDR = 0x60;

static const int UART_SPEED_CONTROLLER = 9600; // 38400;

static const int UART_SPEED_SOFTWARE = 9600;

static const int UART_SPEED_PROJECTOR = 9600;

/**
 * UART command codes
 **/
// Command header
static const uint8_t CMD_HEADER = 0x80;

enum CommandOpcode : uint8_t {
    // Set Light Intensity. Value = 0..127
    CMD_LIGHT_INTENSITY   = 0x01,

    // Set RGB Color. Values = <R> <G> <B>
    CMD_RGB_COLOR         = 0x02,

    // Enable/disable lights. Value = <..|1:RGB on|0:Light on>
    CMD_LIGHT_MODE        = 0x03,

    // Set RGB mode. Value = RGBMode.
    CMD_RGB_MODE          = 0x04,

    // Projector screen control. Value = LiftCommand.
    CMD_SCREEN            = 0x05,

    // Projector mode command. Value = ProjectorMode
    CMD_PROJECTOR_COMMAND = 0x07,

    // Projector lift command. Value = LiftCommand.
    CMD_PROJECTOR_LIFT    = 0x08,

    // Request status. Value = 0
    CMD_REQUEST_STATUS    = 0x09,

    // Read buttons
    CMD_READ_COMMANDS     = 0x10,

    // Read Status.
    CMD_READ_STATUS       = 0x11
};

enum RGBMode : uint8_t {
    // Permanent on
    RGB_ON      = 0x00,
    // RGB color cycle
    RGB_CYCLE   = 0x01,
    // RGB fire flicker
    RGB_FIRE    = 0x02,
    // RGB low light
    RGB_DIMMED  = 0x03
};

enum LiftCommand : uint8_t {
    LIFT_UP   = 0x00,
    LIFT_DOWN = 0x01,
    LIFT_STOP = 0x02
};

enum ProjectorCommand : uint8_t {
    PROJECTOR_OFF    = 0x00,
    PROJECTOR_ON     = 0x01,
    PROJECTOR_NORMAL = 0x02, 
    PROJECTOR_3D     = 0x03,
    PROJECTOR_VR     = 0x04
};
