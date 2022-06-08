/**
 * Project:   FancyLights
 * File:      keypad.c
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
 * Keypad control board sourcecode for FancyLights.
 *
 * Pinout:
 * - PortB:
 *   - PB0: 
 *   - PB1: 
 *   - PB2: 
 *   - PB3: 
 *   - PB4: 
 *   - PB5: 
 *   - PB6: 
 *   - PB7: 
 * - PortC:
 *   - PC0: 
 *   - PC1: 
 *   - PC2: 
 *   - PC3: 
 *   - PC4: 
 *   - PC5: 
 *   - PC6: 
 * - PortD:
 *   - PD0: 
 *   - PD1: 
 *   - PD2:
 *   - PD3:
 *   - PD4: 
 *   - PD5: 
 *   - PD6: 
 *   - PD7: 
 **/

// Define CPU clock speed and UART baud rate
#define F_CPU 1000000
#define BAUD  9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Baud rate calculation, delays
#include <util/setbaud.h>
#include <util/delay.h>

#include "uart.h"
#include "command.h"

#define SET_BIT(port,pin,value) \
    if (value) { \
	port |= _BV(pin);  \
    } else { \
	port &= ~_BV(pin); \
    }

int main()
{    
    // Disable interrupts
    cli();

    // Enable interrupts
    sei();

    while(1) {
	// Wait for any inputs
	//sleep();

	// Debounce inputs
	_delay_ms(25);
    }

    return 0;
}
