/** \file CommandLine.h */
/*
 * NAME: CommandLine.h
 *
 * WHAT:
 *  Header file for command line processing library class and defines.
 *
 * SPECIAL CONSIDERATIONS:
 *  None
 *
 * AUTHOR:
 *  D.L. Karmann
 *
 */
#ifndef __COMMANDLINE_H__
#define __COMMANDLINE_H__

#include "Arduino.h"

#define CLS_HOME    "\033[2J"
#define CHAR_BS     0x08

/**
 *  Defines the maximum number of arguments that can be parsed.
 *  (this includes the command itself plus its parameters)
 */
#define CMDLINE_MAX_ARGS        10

#define CMDLINE_MAX_TERMINATORS 2

#define CMD         0
#define ARG1        1
#define ARG2        2
#define ARG3        3
#define ARG4        4
#define ARG5        5
#define ARG6        6
#define ARG7        7
#define ARG8        8
#define ARG9        9

/**
 *  Defines of values returned when parsing arguments.
 */
#define BADPARAM    -1
#define DECVAL      1
#define HEXVAL      2
#define STRVAL      3

/**
 *  Command line function callback type.
 */
typedef int8_t (* pfnCmdLine)(int8_t argc, char * argv[]);

/**
 *  Command line custom error handling function callback type.
 */
typedef void (* pfnCustomErrs)(int8_t err_code);

/**
 * Structure for an entry in the command list table.
 */
typedef struct
{
    /// A pointer to a string containing the name of the command.
    PGM_P pcCmd;

    /// A function pointer to the implementation of the command.
    pfnCmdLine pfnCmd;

    /// A pointer to a string of brief help text for the command.
    PGM_P pcHelp;
} tCmdLineEntry;

/**
 * This is the command table that must be provided by the application.
 */
extern const tCmdLineEntry g_sCmdTable[] PROGMEM;

/**
 * CommandLine Arduino library class. Version: "V1.10 7/7/2023"
 */
class CommandLine
{
    public:
        /**
         * Defines the size of the buffer that holds the command line.
         */
        #define CMD_BUF_SIZE            80

        /// Defines the value that is returned if the command is not found.
        #define CMDLINE_BAD_CMD         (-1)

        /// Defines the value that is returned if there are too many arguments.
        #define CMDLINE_TOO_MANY_ARGS   (-2)

        /// Defines the value that is returned if there are not enough arguments.
        #define CMDLINE_TOO_FEW_ARGS    (-3)

        /// Defines the value that is returned if an argument is invalid.
        #define CMDLINE_INVALID_ARG     (-4)

        // Constructors
        /**
         *  A constructor that sets up the command line processing code.
         *
         *  \param serial: the stream that a command line is implemented on (typically 'Serial')
         *
         *  \return None.
         *
         *  \note Defaults to enable echo of incoming characters.
         */
        CommandLine(Stream& serial);

        /**
         *  A constructor that sets up the command line processing code.
         *
         *  \param serial: the stream that a command line is implemented on (typically 'Serial')
         *  \param echoEnable: a flag that is used to enable/disable echo of incoming characters
         *                    (\e true = enable echo, \e false = disable echo)
         *
         *  \return None.
         */
        CommandLine(Stream& serial, bool echoEnable);

        /**
         * Implements the non-blocking serial command processing.
         *
         * \return   int8_t
         * \return   0 = no command to process yet
         * \return   1 = a command was processed
         *
         *  \note This should be called in \e 'loop()' to check for/process incoming commands.
         */
        int8_t DoCmdLine(void);

        /**
         * Support routine that parses a command parameter string into a decimal or hex
         * numeric value or identifies it as a quoted string.
         *
         * \param param: parameter string to parse
         * \param retval: place for parsed numeric value (if type DECVAL or HEXVAL)
         *
         * \return   type of parameter
         * \return   - DECVAL   = decimal numeric parameter value (1234) - value returned at *retval
         * \return   - HEXVAL   = hex numeric parameter value (0x12ab) - value returned at *retval
         * \return   - STRVAL   = string parameter value ("quoted string")
         * \return   - BADPARAM = bad parameter
         *
         *  \note The command line processor (\b CmdLineProcess()) does *not* keep the contents
         *  of a quoted string intact. Each word in the string (a space is considered a
         *  word delimiter - TABs are not) is placed in a separate argument! (The argument
         *  with the first word in the quoted string contains the opening quote and the
         *  argument with the last word in the quoted string contains the closing quote.)
         */
        int8_t ParseParam(char * param, int32_t * retval);

        /**
         * Enables/disables echo of incoming characters (default is enabled).
         *
         * \param echoEnable: a flag that is used to enable/disable echo of incoming characters
         *                    (\e true = enable echo, \e false = disable echo)
         *
         *  \note CR/LF will not be echoed if \ref Echo() is enabled and \ref CrLfEcho() is disabled.
         */
        void Echo(bool echoEnable);

        /**
         * Enables/disables echo of incoming CR/LF characters (default is disabled).
         *
         * \param crLfechoEnable: a flag that is used to enable/disable echo of incoming CR/LF characters
         *                        (\e true = enable echo, \e false = disable echo)
         *
         *  \note This has no effect if \ref Echo() is disabled.
         */
        void CrLfEcho(bool crLfechoEnable);

        /**
         * Enables/disables sending CR/LF characters before processing command (default is enabled).
         *
         * \param crLfcmdEnable: a flag that is used to enable/disable sending CR/LF characters
         *                       before processing a command
         *                       (\e true = enable CR/LF response, \e false = disable CR/LF response)
         */
        void CrLfCommand(bool crLfcmdEnable);

        /**
         * Sets the parameter separator character to use (if other than the default space ' ').
         *
         * \param delimiter: the delimiter (or parameter separator) character
         */
        void Delimiter(char delimiter);

        /**
         * Sets the parameter separator character(s) to use (if other than the default '\\r').
         *
         * \param terminators: the command line terminator character(s) to use (2 maximum supported)
         */
        void Terminators(char * terminators);

        /**
         * Sets the custom handler function to use for unknown commands
         * (i.e. commands not in command table) (default is none).
         *
         * \param function: the handler function for unknown commands
         */
        void SetDefaultHandler(pfnCmdLine function);

        /**
         * Sets the custom handler function to use for commands errors (default is internal).
         *
         * \param function: the custom handler function for commands errors
         */
        void SetCustomErrorHandler(pfnCustomErrs function);

        /**
         * Shows the menu commands.
         *
         * \param help_info_disable: optional flag to disable (if \e true) showing help information
         *                           (default = false)
         */
        void ShowCommands(bool help_info_disable = false);

        /**
         * Flush the serial receive buffer.
         */
        void FlushReceive(void)
        {
            input.index = 0;
            input.g_cCmdBuf[0] = '\0';
        }

    private:
        // the I/O stream for the command line characters
        Stream& serial;

        // the buffer that holds the command line (and some input control variables)
        struct
        {
            bool echoEnable;
            bool crLfechoEnable;
            bool crLfcmdEnable;
            uint8_t index;
            char g_cCmdBuf[CMD_BUF_SIZE];
        } input;

        // pointers to command line parameters
        char * argv[CMDLINE_MAX_ARGS];

        // container for command line parameter separator
        char delimiter;

        // container for command line terminator(s)
        char terminators[CMDLINE_MAX_TERMINATORS + 1];

        // pointer to unknown command handler
        pfnCmdLine defaultFunc;

        // pointer to unknown command handler
        pfnCustomErrs errorFunc;

        // sets the operating defaults.
        void SetDefaults(bool echoEnable);

        // processes a command line string into arguments and executes the command
        int8_t CmdLineProcess(char * pcCmdLine);
};

#endif // __COMMANDLINE_H__
