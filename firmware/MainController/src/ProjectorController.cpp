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

#include <functional>

#include <commands.h>

#include "config.h"

HardwareSerial projectorSerial(UART_PROJECTOR);

const char *TOPIC_PROJECTOR_MODE = "mode";
const char *TOPIC_PROJECTOR_MOVE = "move";
const char *TOPIC_SCREEN_MOVE = "screen";
const char *TOPIC_PROJECTOR_POWER = "power";
const char *TOPIC_PROJECTOR_SWITCH = "endstop";

ProjectorController::ProjectorController(Settings &settings, MqttClient &mqttClient)
: mSettings(settings), mMqttClient(mqttClient)
{
}

void ProjectorController::mqttCallback(const char *key, const char* payload, unsigned int length)
{
    if (length == 0) {
        return;
    }
    if (strcmp(key, TOPIC_PROJECTOR_MODE) == 0) {
        ProjectorCommand mode;
        if (parseProjectorCommand(payload, mode)) {
            sendMode(mode);
        }
    }
    if (strcmp(key, TOPIC_PROJECTOR_MOVE) == 0) {
        LiftCommand move;
        if (parseLiftCommand(payload, move)) {
            moveProjector(move);
        }
    }
    if (strcmp(key, TOPIC_SCREEN_MOVE) == 0) {
        LiftCommand move;
        if (parseLiftCommand(payload, move)) {
            moveScreen(move);
        }
    }
}

void ProjectorController::subscribeCallback()
{
    mMqttClient.subscribe(MQS_PROJECTOR, TOPIC_PROJECTOR_MODE);
    mMqttClient.subscribe(MQS_PROJECTOR, TOPIC_PROJECTOR_MOVE);
    mMqttClient.subscribe(MQS_PROJECTOR, TOPIC_SCREEN_MOVE);
}

void ProjectorController::sendMode(ProjectorCommand mode)
{
    mMode = mode;

    // send command via SW UART to projector
    projectorSerial.write(ProjectorOpcode::POP_MODE);
    projectorSerial.write(mode);
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
    // Request status from projector controller
    projectorSerial.write(ProjectorOpcode::POP_STATUS);
}

void ProjectorController::processSerialData(uint8_t data)
{
    mCommandData[mBufferSize++] = data;

    switch (mCommandData[0]) {
        case ProjectorOpcode::POP_STATUS:
            if (mBufferSize >= 2) {
                bool powerOn = ((mCommandData[1]>>1) & 0x01);
                bool switchState = (mCommandData[1] & 0x01);

                if (mStatusCallback) {
                    mStatusCallback(switchState, powerOn);
                }

                mMqttClient.publish(MQS_PROJECTOR, TOPIC_PROJECTOR_POWER, strBool(powerOn));
                mMqttClient.publish(MQS_PROJECTOR, TOPIC_PROJECTOR_SWITCH, strBool(switchState));

                // finish the command
                mBufferSize = 0;
            }
            break;
    }

    if (mBufferSize >= COM_BUF_SIZE) {
        // reset command buffer when full
        mBufferSize = 0;
    }
}

void ProjectorController::begin()
{
    using namespace std::placeholders;

    projectorSerial.begin(UART_SPEED_CONTROLLER, SERIAL_8N1, PIN_PR_RXD, PIN_PR_TXD);

    pinMode(PIN_SCREEN_UP, OUTPUT);
    pinMode(PIN_SCREEN_DOWN, OUTPUT);

    digitalWrite(PIN_SCREEN_UP, LOW);
    digitalWrite(PIN_SCREEN_DOWN, LOW);

    pinMode(PIN_WINCH_SWITCH, INPUT_PULLUP);
    pinMode(PIN_MOTOR_IN1, OUTPUT);
    pinMode(PIN_MOTOR_IN2, OUTPUT);

    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);

    auto callback = std::bind(&ProjectorController::mqttCallback, this, _1, _2, _3);
    auto subscribeCallback = std::bind(&ProjectorController::subscribeCallback, this);

    mMqttClient.registerClient(MQS_PROJECTOR, callback, subscribeCallback);
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

        processSerialData(data);
    }
}
