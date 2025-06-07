/**
 * @project     FancyLights
 * @author      Stefan Hepp, stefan@stefant.org
 *
 * Commandline parser
 *
 * Copyright 2025 Stefan Hepp
 * License: GPL v3
 * See 'COPYRIGHT.txt' for copyright and licensing information.
 */
#pragma once

#include <inttypes.h>

enum CmdErrorCode
{
    // Command successfully completed
    CmdOK,
    // Command expects another argument
    CmdNextArgument,
    // Command processing failed (invalid arguments), either too much or wrong arguments
    CmdInvalidArgument,
    // Command execution failed.
    CmdError
};

class CommandParser
{
    protected:
        bool parseInteger(const char* arg, int &value, int minValue, int maxValue);

    public:
        virtual void printArguments() { }

        virtual void resetCommand() { }
    
        /**
         * Start parsing a new command "cmd".
         * 
         * @return True if there is another argument expected.
         */
        virtual CmdErrorCode startCommand(const char* cmd) = 0;

        virtual CmdErrorCode parseNextArgument(int argNo, const char* arg) { return CmdErrorCode::CmdInvalidArgument; }

        virtual CmdErrorCode completeCommand(bool expectArgument) { return expectArgument ? CmdErrorCode::CmdNextArgument 
                                                                                          : CmdErrorCode::CmdOK; 
                                                                  }
};

static const int MAX_PARSERS = 16;
static const int MAX_TOKEN_LENGTH = 32;

class CommandLine
{
    private:
        CommandParser* mParsers[MAX_PARSERS];
        const char* mCommands[MAX_PARSERS];
        int mNumCommands = 0;

        char mToken[MAX_TOKEN_LENGTH];
        int  mTokenLength = 0;

        int  mCurrentCommand = -1;
        int  mArgumentNo = 0;
        bool mExpectCommand = true;

        void printHelp();

        void printCommandHelp(int cmd);

        void handleRetCode(CmdErrorCode ret, bool eol);

        void selectCommand();

        void processArgument();

        void completeCommand();

        void processToken();

        void processEOL();

    public:
        explicit CommandLine();

        void addCommand(const char* cmd, CommandParser *parser);

        void begin();

        void loop();
};