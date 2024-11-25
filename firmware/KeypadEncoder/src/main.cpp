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
#include "LightState.h"

// Create device driver instances
MegaWire Wire;

Keypad keypad;
LightState LState;

bool LightSwitch;

bool StatusReceived = false;

static const uint8_t IRQ_COMMAND = 0;
static const uint8_t IRQ_STATUS = 1;

static uint16_t IRQFlags = 0x00;

static uint8_t I2CCommand = 0x00;

static const int I2C_BUFFER_SIZE = 16;
static uint8_t I2CBuffer[I2C_BUFFER_SIZE];
static uint8_t I2CBufferLength = 0;

static const int UART_BUFFER_SIZE = 16;
static uint8_t UARTBuffer[UART_BUFFER_SIZE];
static uint8_t UARTBufferLength = 0;

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

void sendCommand(uint8_t command, uint8_t value, bool sendI2C = true) 
{
    Serial.write(CMD_HEADER | command);
    Serial.write(value);

    if (sendI2C) {
        noInterrupts();
        if (I2CBufferLength < I2C_BUFFER_SIZE - 1) {
            I2CBuffer[I2CBufferLength++] = CMD_HEADER | command;
            I2CBuffer[I2CBufferLength++] = value;
        }
        interrupts();
        sendIRQ(IRQ_COMMAND);
    }
}

void sendRGBColor(bool sendI2C = true)
{
    Serial.write(CMD_HEADER | CMD_RGB_COLOR);
    Serial.write(LState.rgb(0));
    Serial.write(LState.rgb(1));
    Serial.write(LState.rgb(2));

    if (sendI2C) {
        noInterrupts();
        if (I2CBufferLength < I2C_BUFFER_SIZE - 3) {
            I2CBuffer[I2CBufferLength++] = CMD_HEADER | CMD_RGB_COLOR;
            I2CBuffer[I2CBufferLength++] = LState.rgb(0);
            I2CBuffer[I2CBufferLength++] = LState.rgb(1);
            I2CBuffer[I2CBufferLength++] = LState.rgb(2);
        }
        interrupts();
        sendIRQ(IRQ_COMMAND);
    }
}

void processCommand(uint8_t data)
{
    if (UARTBufferLength == 0 && (data & CMD_HEADER) != CMD_HEADER) {
        // Wait for command byte, drop other data
        return;
    }

    if (UARTBufferLength < UART_BUFFER_SIZE) {
        UARTBuffer[UARTBufferLength++] = data;
    }

    switch (UARTBuffer[0]) {
        case CMD_READ_STATUS:
            if (UARTBufferLength >= 8) {
                // command completely received
                LState.setLightMode(UARTBuffer[1]);
                LState.setIntensity(UARTBuffer[2]);
                LState.setProjectorMode(UARTBuffer[3]);
                LState.setRGBMode(UARTBuffer[4]);
                LState.setRGB(UARTBuffer[5], UARTBuffer[6], UARTBuffer[7]);

                UARTBufferLength = 0;
                StatusReceived = true;
            }
            break;
        default:
            // unknown command, drop
            UARTBufferLength = 0;
            break;
    }
}

void onButtonPress(uint8_t btn, bool longPress)
{
    sendCommand(1, btn | (longPress ? 0x80 : 0) );

    switch (btn) {
        case 0:

            break;
        case 1:

            break;
        case 2:
        
            break;
        case 3:
        
            break;
        case 4:
        
            break;
        case 5:
        
            break;
        case 6:
        
            break;
        case 7:
        
            break;
        case 8:
        
            break;
        case 9:
        
            break;
        case 10:
        
            break;
        case 11:
        
            break;
        case 12:
        
            break;
        case 13:
        
            break;
        case 14:
        
            break;
        case 15:
        
            break;
    }
}

void i2cReceive(uint8_t length) {
    uint8_t I2CCommand = Wire.read();
    uint8_t value;

    switch (I2CCommand) {
        case CommandOpcode::CMD_LIGHT_MODE:
        case CommandOpcode::CMD_LIGHT_INTENSITY:        
        case CommandOpcode::CMD_RGB_MODE:
        case CommandOpcode::CMD_SCREEN:
        case CommandOpcode::CMD_PROJECTOR_COMMAND:
        case CommandOpcode::CMD_PROJECTOR_LIFT:
            value = Wire.read();

            // Update local state
            switch (I2CCommand) {
                case CommandOpcode::CMD_LIGHT_MODE:
                    LState.setLightMode(value);
                    break;
                case CommandOpcode::CMD_LIGHT_INTENSITY:
                    LState.setIntensity(value);
                    break;       
                case CommandOpcode::CMD_RGB_MODE:
                    LState.setRGBMode(value);
                    break;
                case CommandOpcode::CMD_PROJECTOR_COMMAND:
                    LState.setProjectorMode(value);
                    break;
            }

            // Forward command to controller
            sendCommand(I2CCommand, value, false);
            break;
        case CommandOpcode::CMD_RGB_COLOR:
            if (Wire.available() > 2) {
                LState.setRGB(Wire.read(), 
                              Wire.read(), 
                              Wire.read());
                sendRGBColor(false);
            }
            break;
        case CommandOpcode::CMD_REQUEST_STATUS:
            clearIRQ(IRQ_STATUS);
            sendCommand(I2CCommand, 0, false);
            break;
        case CommandOpcode::CMD_READ_COMMANDS:
        case CommandOpcode::CMD_READ_STATUS:
            // expect i2c read request after this write
            break;
            break;
    }
}

void i2cRequest()
{
    switch (I2CCommand) {
        case CommandOpcode::CMD_READ_COMMANDS:
            Wire.write(I2CBufferLength | (IRQFlags & (1<<IRQ_STATUS) ? 0x80 : 0) );

            for (uint8_t i = 0; i < I2CBufferLength; i++) {
                Wire.write(I2CBuffer[i]);
            }

            I2CBufferLength = 0;

            clearIRQ(IRQ_COMMAND);
            break;
        case CommandOpcode::CMD_READ_STATUS:
            Wire.write(LState.lightMode());
            Wire.write(LState.lightIntensity());
            Wire.write(LState.projectorMode());
            Wire.write(LState.rgbMode());
            Wire.write(LState.rgb(0));
            Wire.write(LState.rgb(1));
            Wire.write(LState.rgb(2));

            clearIRQ(IRQ_STATUS);
            break;
    }
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

    Serial.begin(UART_SPEED_CONTROLLER);

    Wire.begin(KEYPAD_I2C_ADDR);
    keypad.begin();

    StatusReceived = false;
    delay(500);
    sendCommand(CMD_REQUEST_STATUS, 0);
}

void loop() {
    if (StatusReceived) {
        keypad.poll();

        uint8_t newSwitchState = digitalRead(PIN_SWITCH);
        if (newSwitchState != LightSwitch) {
            LState.toggleLight();

            sendCommand(CMD_LIGHT_MODE, LState.lightMode());

            LightSwitch = newSwitchState;
        }
    }

    while (Serial.available()) {
        int data = Serial.read();

        processCommand(data);
    }
}
