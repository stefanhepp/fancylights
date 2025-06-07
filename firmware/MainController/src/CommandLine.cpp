/*
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Cmdline parser implementation
 *
 * Copyright 2025 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#include "CommandLine.h"

#include <Arduino.h>

#include <inttypes.h>
#include <cstring>
#include <string>

bool CommandParser::parseInteger(const char* arg, int &value, int minValue, int maxValue)
{
    const char* c = arg;
    while (*c != '\0') {
        if (*c < '0' || *c > '9') {
            return false;
        }
        c++;
    }

    value = std::stoi(arg);

    if (value < minValue || value > maxValue) {
        return false;
    }

    // conversion and range check successful
    return true;
}

CommandLine::CommandLine()
{
}

void CommandLine::addCommand(const char* cmd, CommandParser* parser)
{
    if (mNumCommands >= MAX_PARSERS) {
        return;
    }
    mCommands[mNumCommands] = cmd;
    mParsers[mNumCommands] = parser;
    mNumCommands++;
}

void CommandLine::printCommandHelp(int cmd)
{
    Serial.print(mCommands[cmd]);
    Serial.print(' ');
    mParsers[cmd]->printArguments();
    Serial.println();
}

void CommandLine::printHelp()
{
    for (int i = 0; i < mNumCommands; i++) {
        printCommandHelp(i);
    }
}

void CommandLine::handleRetCode(CmdErrorCode ret, bool eol)
{
    switch (ret) {
        case CmdErrorCode::CmdOK:
            if (eol) {
                Serial.println("OK");
            }
            // no arguments expected
            mExpectCommand = false;
            break;
        case CmdErrorCode::CmdNextArgument:
            if (eol) {
                Serial.print("Missing arguments: ");
                printCommandHelp(mCurrentCommand);
            }
            mExpectCommand = true;
            break;
        case CmdErrorCode::CmdInvalidArgument:
            Serial.print("Error parsing command: ");
            Serial.println(mToken);

            mExpectCommand = false;
            mCurrentCommand = -1;
            break;
        case CmdErrorCode::CmdError:
            Serial.println("FAIL");

            mExpectCommand = false;
            mCurrentCommand = -1;
            break;
    }
}

void CommandLine::selectCommand()
{
    for (int i = 0; i < mNumCommands; i++) {
        if (strcmp(mToken, mCommands[i]) == 0) {
            // Found a valid command
            mCurrentCommand = i;
            mArgumentNo = 0;

            CmdErrorCode ret = mParsers[i]->startCommand(mToken);

            // OK, continue, fail?
            handleRetCode(ret, false);

            // found a command, abort
            return;
        }
    }

    if (strcmp(mToken, "help") == 0) {
        printHelp();
        return;
    }

    // only reached if no command matched
    Serial.printf("Invalid command '%s'! See 'help'.\n", mToken);

    mExpectCommand = false;
    mCurrentCommand = -1;
}

void CommandLine::processArgument()
{
    if (mCurrentCommand == -1) {
        return;
    }

    if (mTokenLength > 0) {    
       CmdErrorCode ret = mParsers[mCurrentCommand]->parseNextArgument(mArgumentNo++, mToken);

        // OK, continue, fail?
        handleRetCode(ret, false);
    }
}

void CommandLine::completeCommand()
{
    if (mCurrentCommand != -1) {
        CmdErrorCode ret = mParsers[mCurrentCommand]->completeCommand(mExpectCommand);
        handleRetCode(ret, true);

        if (ret != CmdErrorCode::CmdOK) {
            mParsers[mCurrentCommand]->resetCommand();
        }

        mExpectCommand = false;
        mCurrentCommand = -1;
    }
}

void CommandLine::processToken()
{
    if (mExpectCommand) {
        if (mCurrentCommand == -1) {
            // We are starting a new command
            selectCommand();
        } else {
            // Got another argument
            processArgument();
        }

    } else {
        if (mCurrentCommand != -1) {
            // Received another argument that the command was not expecting
            Serial.print("Ignoring unexpected argument: ");
            Serial.print(mToken);
            Serial.println();

            mCurrentCommand = -1;

        } else {
            // Silently ignore any other arguments. Error was already reported.
        }
    }
}

void CommandLine::processEOL()
{
    if (mCurrentCommand != -1) {
        completeCommand();       
    }
    mExpectCommand = true;
    mCurrentCommand = -1;
}

void CommandLine::begin()
{
    // Serial over USB.
    Serial.begin(115200);
}

void CommandLine::loop()
{
    while (Serial.available()) {
        char c = Serial.read();

        // echo inputs
        Serial.write(c);
        Serial.flush();

        switch (c) {
            case '\r':
                break;
            case ' ':
            case '\t':
            case '\n':
                if (mTokenLength > 0) {
                    // zero-terminate token
                    if (mTokenLength < MAX_TOKEN_LENGTH) {
                        mToken[mTokenLength++] = '\0';
                    } else {
                        // Forcefully terminate at the end. Should never be reached.
                        mToken[MAX_TOKEN_LENGTH-1] = '\0';
                    }

                    processToken();
                    
                    mTokenLength = 0;
                }
                if (c == '\n') {
                    processEOL();
                }
                break;
            case 8: // Backspace
                if (mTokenLength > 0) {
                    mTokenLength--;
                }
                break;
            default:
                // We leave one byte space in the buffer for zero-terminiation
                if (mTokenLength < MAX_TOKEN_LENGTH - 1) {
                    mToken[mTokenLength++] = c;
                } else {
                    // raise error?
                }
                break;
        }
    }
} 