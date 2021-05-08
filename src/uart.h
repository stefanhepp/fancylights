/**
 * Project:   LightControl
 * File:      uart.h
 * Author:    Stefan Hepp <stefan@stefant.org>
 *
 * Copyright 2020 Stefan Hepp
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
 * Common UART interface functions and structures.
 *
 */
#ifndef __LC_UART_H__
#define __LC_UART_H__

// UART buffer size in characters
#define UART_BUF_SZ  8

// UART send/receive buffer
typedef struct {
    unsigned char start;
    unsigned char len;
    unsigned char buf[UART_BUF_SZ];
} uart_buf_t;


#endif
