/*
 * @project     FancyController
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * LED intensity and mode implementation.
 *
 * Copyright 2024 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "ProjectorController.h"

#include <commands.h>

#include "config.h"

HardwareSerial projectorSerial(UART_PROJECTOR);

ProjectorController::ProjectorController(Settings &settings)
: mSettings(settings)
{
}

void ProjectorController::setMode(ProjectorCommand mode)
{
    // TODO send command via SW UART to projector

    mMode = mode;
}

void ProjectorController::moveProjector(LiftCommand direction)
{
    
}

void ProjectorController::moveScreen(LiftCommand direction)
{
    mCntPulse = 50;
    if (direction == LiftCommand::LIFT_UP || direction == LiftCommand::LIFT_STOP) {
        digitalWrite(PIN_SCREEN_UP, HIGH);
    }
    if (direction == LiftCommand::LIFT_DOWN || direction == LiftCommand::LIFT_STOP) {
        digitalWrite(PIN_SCREEN_DOWN, HIGH);
    }
}

void ProjectorController::requestStatus()
{

}

void ProjectorController::begin()
{
    projectorSerial.begin(UART_SPEED_PROJECTOR, SERIAL_8N1, PIN_PR_RXD, PIN_PR_TXD);

    pinMode(PIN_SCREEN_UP, OUTPUT);
    pinMode(PIN_SCREEN_DOWN, OUTPUT);

    digitalWrite(PIN_SCREEN_UP, LOW);
    digitalWrite(PIN_SCREEN_DOWN, LOW);

    pinMode(PIN_WINCH_SWITCH, INPUT_PULLUP);
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);

    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
}

void ProjectorController::loop()
{
    if (mCntPulse > 0) {
        mCntPulse--;
        if (mCntPulse == 0) {
            digitalWrite(PIN_SCREEN_UP, LOW);
            digitalWrite(PIN_SCREEN_DOWN, LOW);
        }
    }

    // Process UART response
    while (projectorSerial.available()) {
        char data = projectorSerial.read();

        //processCommand(data);
    }
}
