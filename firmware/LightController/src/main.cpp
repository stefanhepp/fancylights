/**
 * Author: Stefan Hepp (stefan@stefant.org)
 * 
 * Implementation of the pedalboard controller.
 * Reads pedal switches and generates MIDI messages.
 * Configurable either via config switch or I2C interface.
 * 
 * Pins:
 * - PB0-7: IN;  Keyboard data bytes
 * - PC3:   OUT; Interrupt to signal I2C data ready
 * - PC4,5: I2C
 * - PC6:   In;  Reset (via ISP)
 * - PD0,1  UART
 * - PD2-4: OUT; Select line
 * - PD5-6: OUT; Select keyboard
 **/

#include <Arduino.h>
#include <MegaWire.h>

#include <commands.h>

#include "config.h"
#include "Keypad.h"

// Create device driver instances
MegaWire Wire;

Keypad keypad;

static const uint8_t IRQ_BUTTONS = 0;

static uint16_t IRQFlags = 0x00;

static void sendIRQ(uint8_t flag)
{
    IRQFlags |= (1<<flag);
    digitalWrite(PIN_INTERRUPT, HIGH);
}

static void clearIRQ(uint8_t flag)
{
    IRQFlags &= ~(1<<flag);
    if (IRQFlags == 0x00) {
        digitalWrite(PIN_INTERRUPT, LOW);
    }
}

void onButtonPress(uint8_t btn, bool longPress)
{

}

void i2cReceive(uint8_t length) {
    uint8_t cmd = Wire.read();

    switch (cmd) {
        case I2C_CMD_SET_LEDS:
            if (Wire.available() > 3) {
                uint8_t mask1 = Wire.read();
                uint8_t mask2 = Wire.read();
                uint8_t mask3 = Wire.read();
                uint8_t mask4 = Wire.read();
                uint8_t kbd = (mask1 >> 7);
                pistons.setLEDs(kbd, mask1 & 0x7F, mask2, mask3, mask4);
            }
            break;
    }
}

void i2cRequest()
{

    clearIRQ(IRQ_BUTTONS);
}

void setup() {
    // Enable pullups for unconnected pins
    pinMode(PIN_PB1, INPUT_PULLUP);
    pinMode(PIN_PB2, INPUT_PULLUP);
    pinMode(PIN_PB3, INPUT_PULLUP);
    pinMode(PIN_PB4, INPUT_PULLUP);
    pinMode(PIN_PB5, INPUT_PULLUP);
    pinMode(PIN_PB6, INPUT_PULLUP);
    pinMode(PIN_PB7, INPUT_PULLUP);
    pinMode(PIN_PC6, INPUT_PULLUP);
    pinMode(PIN_PD3, INPUT_PULLUP);

    pinMode(PIN_SWITCH, INPUT_PULLUP);

    // Set output pin modes
    // Set pin value first before turing on output mode, to prevent spurious signals
    digitalWrite(PIN_INTERRUPT, LOW);
    pinMode(PIN_INTERRUPT, OUTPUT);

    Wire.onReceive(i2cReceive);
    Wire.onRequest(i2cRequest);
    keypad.setPressEvent(onButtonPress);

    Wire.begin(KEYPAD_I2C_ADDR);
    keypad.begin();
}

void loop() {
    keypad.poll();
}
