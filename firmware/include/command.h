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
#ifndef __COMMAND_H__
#define __COMMAND_H__

/**
 * UART command codes
 **/
// Command header
#define UART_CMD_HEADER        0x70

// Command Opcodes:
// - Living room light. Value = 0: Off, 1: Light on, 2: Long press
#define UART_CMD_LIVINGROOM    0x01
// - Stair case light. Value = 0: Off, 1: Light on, 2: Stair long press
#define UART_CMD_STAIRCASE     0x02
// - Stair light. Value = 000000 | Stair Sense | Stair Light
#define UART_CMD_STAIR         0x03
// - All off. Value = 1: All off
#define UART_CMD_ALL_OFF       0x04
// - Status. Value = 000 | Sense | Stair Sense | Stair Light | Staircase | Livingroom
#define UART_CMD_STATUS        0x05
// - Sense notification. Value = 0: Motion Timeout, 1: Motion detected
#define UART_CMD_SENSE         0x06


#endif
