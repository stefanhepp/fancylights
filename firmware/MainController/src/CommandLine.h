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

enum CmdParseStatus
{
    // All mandatory arguments parsed.
    CPSComplete,
    // Command expects another argument
    CPSNextArgument,
    // Command processing failed (invalid arguments), either too much or wrong arguments
    CPSInvalidArgument
};

enum CmdExecStatus
{
    // Command successfully executed
    CESOK,
    // Command processing failed (invalid arguments), either too much or wrong arguments
    CESInvalidArgument,
    // Command execution failed.
    CESError
};

class CommandParser
{
    protected:
        bool parseInteger(const char* arg, int &value, int minValue, int maxValue);

    public:
        /** 
         * Print the command line arguments
         */
        virtual void printArguments() { }

        /**
         * Called when the command was aborted because of some error.
         */
        virtual void resetCommand() { }
    
        /**
         * Start parsing a new command "cmd".
         * 
         * @return CmdNextArgument if there are mandatory arguments, or CmdOK if there are only optional arguments.
         */
        virtual CmdParseStatus startCommand(const char* cmd) { return CPSComplete; };

        virtual CmdParseStatus parseNextArgument(int argNo, const char* arg) { return CPSInvalidArgument; }

        /**
         * Called when all arguments are parsed, execute the command here.
         * 
         * @param expectArgument: if a previous call returned CmdNextArgument, otherwise false.
         * @return: CmdOK if command was successful, otherwise CmdError or CmdInvalidArgument.
         */
        virtual CmdExecStatus completeCommand(bool expectArgument) = 0;
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
        // Waiting for a new command?
        bool mExpectCommand = true;
        // Waiting for an argument?
        bool mExpectArgument = false;

        void printHelp();

        void printCommandHelp(int cmd);

        void handleParseStatus(CmdParseStatus ret);

        void handleExecStatus(CmdExecStatus ret);

        void selectCommand();

        void processArgument();

        void completeCommand();

        void abortCommand();

        void processToken();

        void processEOL();

    public:
        explicit CommandLine();

        void addCommand(const char* cmd, CommandParser *parser);

        void begin();

        void loop();
};