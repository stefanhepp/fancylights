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

void CommandLine::handleParseStatus(CmdParseStatus ret)
{
    switch (ret) {
        case CmdParseStatus::CPSComplete:
            // no arguments expected
            mExpectArgument = false;
            break;
        case CmdParseStatus::CPSNextArgument:
            mExpectArgument = true;
            break;
        case CmdParseStatus::CPSInvalidArgument:
            Serial.print("Invalid argument: ");
            printCommandHelp(mCurrentCommand);

            abortCommand();
            break;
    }
}

void CommandLine::handleExecStatus(CmdExecStatus ret)
{
    switch (ret) {
        case CmdExecStatus::CESOK:
            Serial.println("OK");
            break;
        case CmdExecStatus::CESInvalidArgument:
            Serial.print("Invalid arguments for command ");
            Serial.println(mCommands[mCurrentCommand]);
            break;
        case CmdExecStatus::CESError:
            Serial.println("FAIL");
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
            // Parse till we get an error or EOL
            mExpectCommand = false;

            CmdParseStatus ret = mParsers[i]->startCommand(mToken);

            // OK, continue, fail?
            handleParseStatus(ret);

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

    mExpectCommand = true;
    mExpectArgument = false;
    mCurrentCommand = -1;
}

void CommandLine::processArgument()
{
    if (mCurrentCommand == -1) {
        return;
    }

    if (mTokenLength > 0) {    
       CmdParseStatus ret = mParsers[mCurrentCommand]->parseNextArgument(mArgumentNo++, mToken);

        // OK, continue, fail?
        handleParseStatus(ret);
    }
}

void CommandLine::completeCommand()
{
    if (mCurrentCommand != -1) {
        CmdExecStatus ret = mParsers[mCurrentCommand]->completeCommand(mExpectArgument);
        handleExecStatus(ret);

        if (ret != CmdExecStatus::CESOK) {
            abortCommand();
        }
    }
}

void CommandLine::abortCommand()
{
    if (mCurrentCommand != -1) {
        mParsers[mCurrentCommand]->resetCommand();
    }

    mExpectArgument = false;
    mExpectCommand = true;
    mCurrentCommand = -1;
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
    mExpectArgument = false;
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