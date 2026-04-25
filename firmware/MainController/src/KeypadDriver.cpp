/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Keypad interface driver implemention.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */

#include "KeypadDriver.h"

#include <commands.h>

#include "config.h"

HardwareSerial keypadSerial(UART_KEYPAD);

KeypadDriver::KeypadDriver(Settings &settings, LEDDriver &leds, ProjectorController &projector)
: mSettings(settings), mLEDs(leds), mProjector(projector)
{
}

void KeypadDriver::startStatusTimeout() {
    CntStatusTimeout = 80;
}

void KeypadDriver::sendStatus() {
    CntStatusTimeout = 1;
}

bool KeypadDriver::checkStatusTimeout() {
    if (CntStatusTimeout > 0) {
        CntStatusTimeout--;
        if (CntStatusTimeout == 0) {
            // Timeout reached, send status
            return true;
        }
    }
    // no timer running or not yet reached
    return false;
}

void KeypadDriver::begin()
{
    keypadSerial.begin(UART_SPEED_CONTROLLER, SERIAL_8N1, PIN_KP_RXD, PIN_KP_TXD);
}

/**
 * Send the current settings to the keypad controller.
 */
void KeypadDriver::sendKeypadStatus()
{
    keypadSerial.write(CMD_HEADER | CMD_READ_STATUS);
    keypadSerial.write((mLEDs.isLEDStripEnabled() << 1) | (mLEDs.isLampEnabled()));
    keypadSerial.write(mLEDs.intensity());
    keypadSerial.write(mLEDs.dimmedIntensity());
    keypadSerial.write(mProjector.mode());
    keypadSerial.write(mLEDs.rgbMode());
    keypadSerial.write(mLEDs.getHSV().hue);
    keypadSerial.write(mLEDs.getHSV().sat);
    keypadSerial.write(mLEDs.getHSV().val);
}

void KeypadDriver::processKeypadCommand(uint8_t data)
{
    if (UARTBufferLength == 0 && (data & CMD_HEADER) != CMD_HEADER) {
        // Wait for command byte, drop other data
        return;
    }

    if (UARTBufferLength < UART_BUFFER_SIZE) {
        UARTBuffer[UARTBufferLength++] = data;
    }

    switch (UARTBuffer[0] & ~CMD_HEADER) {
        case CMD_LIGHT_INTENSITY:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Set light intensity: %hhu\n", UARTBuffer[1]);
                mLEDs.setIntensity(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_DIMMED_INTENSITY:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Set dimmed intensity: %hhu\n", UARTBuffer[1]);
                mLEDs.setDimmedIntensity(UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_HSV_COLOR:
            if (UARTBufferLength >= 4) {
                Serial.printf("[Kbd] Set HSV: %hhu %hhu %hhu\n", UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
                mLEDs.setHSV(UARTBuffer[1], UARTBuffer[2], UARTBuffer[3]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_LIGHT_MODE:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Enable lamp: %s, LED strip: %s\n", strBool(UARTBuffer[1] & 0x01), strBool(UARTBuffer[1] & 0x02));
                mLEDs.enableLamps(UARTBuffer[1] & 0x01);
                mLEDs.enableLEDStrip(UARTBuffer[1] & 0x02);
                UARTBufferLength = 0;
            }
            break;
        case CMD_RGB_MODE:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Set RGB mode: %s\n", strRGBMode((RGBMode) UARTBuffer[1]));
                mLEDs.setRGBMode((RGBMode) UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_SCREEN:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Move Screen: %s\n", strLiftCommand((LiftCommand) UARTBuffer[1]));
                mProjector.moveScreen((LiftCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_PROJECTOR_MODE:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Set projector mode: %s\n", strProjectorCommand((ProjectorCommand) UARTBuffer[1]));
                mProjector.sendMode((ProjectorCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_PROJECTOR_LIFT:
            if (UARTBufferLength >= 2) {
                Serial.printf("[Kbd] Lift projector: %s\n", strLiftCommand((LiftCommand) UARTBuffer[1]));
                mProjector.moveProjector((LiftCommand) UARTBuffer[1]);
                UARTBufferLength = 0;
            }
            break;
        case CMD_REQUEST_STATUS:
            if (UARTBufferLength >= 2) {
                mProjector.requestStatus();
                startStatusTimeout();
                UARTBufferLength = 0;
            }
            break;
        default:
            // unknown command, drop
            UARTBufferLength = 0;
            break;
    }
}

void KeypadDriver::loop()
{
    // Process keypad UART cmds
    while (keypadSerial.available()) {
        char data = keypadSerial.read();

        processKeypadCommand(data);
    }

    EVERY_N_MILLISECONDS( 10 ) {
        // Check if we need to send status to keypad
        bool timeout = checkStatusTimeout();
        bool changed = mSettings.hasChanged();
        
        if (timeout || changed) {
            sendKeypadStatus();
        }

        mSettings.clearChanged();
    }

}
