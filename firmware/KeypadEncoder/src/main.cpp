/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Keypad main loop implementation.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
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

bool LightSwitch = true;

bool StatusReceived = false;

// Toggle projector power after receiving the status from the controller back
bool ToggleProjectorPowerOnStatus = false;

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
    switch (command) {
        case CommandOpcode::CMD_LIGHT_MODE:
            LState.setLightMode(value);
            break;
        case CommandOpcode::CMD_LIGHT_INTENSITY:
            LState.setIntensity(value);
            break;       
        case CommandOpcode::CMD_RGB_MODE:
            LState.setRGBMode(value);
            break;
        case CommandOpcode::CMD_PROJECTOR_MODE:
            LState.setProjectorMode(value);
            break;
    }

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

void requestStatus() {
    clearIRQ(IRQ_STATUS);
    sendCommand(CommandOpcode::CMD_REQUEST_STATUS, 0, false);
}

void sendHSVColor(bool sendI2C = true)
{
    Serial.write(CMD_HEADER | CMD_HSV_COLOR);
    Serial.write(LState.hsv(0));
    Serial.write(LState.hsv(1));
    Serial.write(LState.hsv(2));

    requestStatus();
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

    switch (UARTBuffer[0] & ~CMD_HEADER) {
        case CMD_READ_STATUS:
            if (UARTBufferLength >= 9) {
                // command completely received
                LState.setLightMode(UARTBuffer[1]);
                LState.setIntensity(UARTBuffer[2]);
                LState.setDimmedIntensity(UARTBuffer[3]);
                LState.setProjectorMode(UARTBuffer[4]);
                LState.setRGBMode(UARTBuffer[5]);
                LState.setHSV(UARTBuffer[6], UARTBuffer[7], UARTBuffer[8]);

                UARTBufferLength = 0;
                StatusReceived = true;

                // We got a new status, inform RPi to fetch the new status
                sendIRQ(IRQ_STATUS);

                if (ToggleProjectorPowerOnStatus) {
                    // If this status was requested by a Projector power toggle button, perform the action now.
                    LState.toggleProjector();
                    sendCommand(CMD_PROJECTOR_MODE, LState.projectorMode());
                    
                    ToggleProjectorPowerOnStatus = false;
                }
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
    switch (btn) {
        case 0: // 3D mode
            sendCommand(CMD_PROJECTOR_MODE, PROJECTOR_3D);
            sendCommand(CMD_RGB_MODE, RGB_DIMMED);
            break;
        case 1: // VR mode
            sendCommand(CMD_PROJECTOR_MODE, PROJECTOR_VR);
            sendCommand(CMD_RGB_MODE, RGB_ON);
            break;
        case 2: // Color cycle forward
            if (longPress) {
                sendCommand(CMD_RGB_MODE, RGB_CYCLE);
            } else if (LState.rgbMode() != RGB_ON) {
                sendCommand(CMD_RGB_MODE, RGB_ON);
            } else {
                LState.changeHSVHue(16);
                sendHSVColor();
            }
            break;
        case 3: // Color cycle back
            if (longPress) {
                sendCommand(CMD_RGB_MODE, RGB_CYCLE);
            } else if (LState.rgbMode() != RGB_ON) {
                sendCommand(CMD_RGB_MODE, RGB_ON);
            } else {
                LState.changeHSVHue(-16);
                sendHSVColor();        
            }
            break;
        case 4: // Fire mode
            sendCommand(CMD_RGB_MODE, RGB_FIRE);        
            break;
        case 5: // Movie mode
            sendCommand(CMD_PROJECTOR_MODE, PROJECTOR_NORMAL);
            sendCommand(CMD_RGB_MODE, longPress ? RGB_ON : RGB_DIMMED);
            break;
        case 6: // RGB saturation down
            LState.changeHSVSaturation(longPress ? -255 : -16);
            sendHSVColor();
            break;
        case 7: // RGB saturation up
            LState.changeHSVSaturation(longPress ? 255 : 16);
            sendHSVColor();
            break;
        case 8: // Screen down
            if (longPress) {
                sendCommand(CMD_SCREEN, LIFT_STOP);
            } else {
                sendCommand(CMD_SCREEN, LIFT_DOWN);
            }        
            break;
        case 9: // Screen up
            if (longPress) {
                sendCommand(CMD_SCREEN, LIFT_STOP);
            } else {
                sendCommand(CMD_SCREEN, LIFT_UP);
            }        
            break;
        case 10: // Intensity down
            LState.changeIntensity(longPress ? -255 : -16);
            sendCommand(CMD_LIGHT_INTENSITY, LState.lightIntensity());
            break;
        case 11: // Intensity up
            LState.changeIntensity(longPress ? 255 : 16);
            sendCommand(CMD_LIGHT_INTENSITY, LState.lightIntensity());
            break;
        case 12: // Projector down
            if (longPress) {
                sendCommand(CMD_PROJECTOR_LIFT, LIFT_STOP);    
            } else {
                sendCommand(CMD_PROJECTOR_LIFT, LIFT_DOWN);
            }        
            break;
        case 13: // Projector up
            if (longPress) {
                sendCommand(CMD_PROJECTOR_LIFT, LIFT_STOP);    
            } else {
                sendCommand(CMD_PROJECTOR_LIFT, LIFT_UP);
            }
            break;
        case 14: // Projector on/off
            // Read the status of the projector first, Delay toggling till we receive the message
            ToggleProjectorPowerOnStatus = true;
            sendCommand(CMD_REQUEST_STATUS, 0);
            break;
        case 15: // Light on/off
            LState.toggleLight();
            sendCommand(CMD_LIGHT_MODE, LState.lightMode());
            break;
    }
}

void i2cReceive(uint8_t length) {
    uint8_t I2CCommand = Wire.read();
    uint8_t value;

    switch (I2CCommand) {
        case CommandOpcode::CMD_LIGHT_MODE:
        case CommandOpcode::CMD_LIGHT_INTENSITY:
        case CommandOpcode::CMD_DIMMED_INTENSITY:
        case CommandOpcode::CMD_RGB_MODE:
        case CommandOpcode::CMD_SCREEN:
        case CommandOpcode::CMD_PROJECTOR_MODE:
        case CommandOpcode::CMD_PROJECTOR_LIFT:
            value = Wire.read();

            // Forward command to controller
            sendCommand(I2CCommand, value, false);
            break;
        case CommandOpcode::CMD_HSV_COLOR:
            if (Wire.available() > 2) {
                LState.setHSV(Wire.read(), 
                              Wire.read(), 
                              Wire.read());
                sendHSVColor(false);
            }
            break;
        case CommandOpcode::CMD_REQUEST_STATUS:
            requestStatus();
            break;
        case CommandOpcode::CMD_READ_COMMANDS:
        case CommandOpcode::CMD_READ_STATUS:
            // expect i2c read request after this write
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
            Wire.write(LState.dimmedIntensity());
            Wire.write(LState.projectorMode());
            Wire.write(LState.rgbMode());
            Wire.write(LState.hsv(0));
            Wire.write(LState.hsv(1));
            Wire.write(LState.hsv(2));

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

    StatusReceived = true;
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
