/*
 * NAME: CommandLine.cpp
 *
 * WHAT:
 *  Command line processing library code.
 *
 * SPECIAL CONSIDERATIONS:
 *  None
 *
 * AUTHOR:
 *  D.L. Karmann
 *
 * MODIFIED:
 *  1/31/2017: "V1.0 1/31/2017"
 *    - Original version
 *  2/3/2017: "V1.1 2/3/2017"
 *    - added initial ESP8266 support
 *  6/8/2019: "V1.2 6/8/2019"
 *    - added more ESP8266 support for updated Arduino ESP8266 compiler
 *  11/2/2020: "V1.3 11/2/2020"
 *    - added ESP32 support
 *  8/19/2021: "V1.4 8/19/2021"
 *    - corrected handling of spaces when delimiter character is *not* a space
 *  4/2/2022: "V1.5 4/2/2022"
 *    - added 32-bit Teensy support
 *  4/6/2022: "V1.6 4/6/2022"
 *    - added support for negative number parameters
 *    - added support for case-insensitive commands
 *  4/8/2022: "V1.7 4/8/2022"
 *    - added support for custom commands errors handler to disable internal display of errors
 *  11/8/2022: "V1.8 11/9/2022"
 *    - added support for control of disable of sending CRLF before processing command
 *  1/24/2023: "V1.9 1/24/2023"
 *    - added initial Raspberry Pi Pico support
 *  7/7/2023: "V1.10 7/7/2023"
 *    - added support for 9 parameters
 */

#include "Arduino.h"
#include "CommandLine.h"

/*
 * NAME:
 *  CommandLine(Stream& _serial)
 *
 * PARAMETERS:
 *  Stream& _serial = the stream that a command line is implemented on (typically 'Serial')
 *
 * WHAT:
 *  A constructor that sets up the command line processing code.
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  Defaults to enable echo of incoming characters.
 */
CommandLine::CommandLine(Stream& _serial) : CommandLine::CommandLine(_serial, true)
{
    // uses other constructor for initialization using "constructor delegation"
}

/*
 * NAME:
 *  CommandLine(Stream& _serial, bool _echoEnable)
 *
 * PARAMETERS:
 *  Stream& _serial = the stream that a command line is implemented on (typically 'Serial')
 *  bool _echoEnable = a flag that is used to enable/disable echo of incoming characters
 *                     (true = enable echo, false = disable echo)
 *
 * WHAT:
 *  A constructor that sets up the command line processing code.
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
CommandLine::CommandLine(Stream& _serial, bool _echoEnable) : serial(_serial)
{
    SetDefaults(_echoEnable);
}

// Sets the operating defaults.
void CommandLine::SetDefaults(bool _echoEnable)
{
    input.echoEnable = _echoEnable;     // specified incoming character echo (changed with Echo())
    input.crLfechoEnable = false;       // default CR/LF echo is off         (changed with CrLfEcho())
    input.crLfcmdEnable = true ;        // default sending CR/LF is on       (changed with CrLfCommand())
    delimiter = ' ';                    // default parameter delimiter       (changed with Delimiter())
    strcpy(terminators, "\r");          // default command line terminator   (changed with Terminators())
    defaultFunc = NULL;                 // default unknown command handler is none (changed with SetDefaultHandler())
    errorFunc = NULL;                   // default command error handler is none (changed with SetCustomErrorHandler())
    input.index = 0;
}

/*
 * NAME:
 *  int8_t DoCmdLine(void)
 *
 * PARAMETERS:
 *  None.
 *
 * WHAT:
 *  Implements the non-blocking serial command processing.
 *
 * RETURN VALUES:
 *  int8_t = 0 = no command to process yet
 *           1 = a command was processed
 *
 * SPECIAL CONSIDERATIONS:
 *  This should be called in 'loop' to check for/process incoming commands.
 */
int8_t CommandLine::DoCmdLine(void)
{
    int nStatus;
    char ch = '\0';

    if (!serial.available())
    {
        return 0;       // no command to process yet
    }
    else
    {
        //
        // Get any available text from the user.
        //
        for (uint8_t i = input.index; i < (sizeof(input.g_cCmdBuf) - 1); ++i)
        {
            if (serial.available())
            {
                ch = (serial.read() & 0x7f);
                if (input.echoEnable)
                {
                    if (((ch != '\r') && (ch != '\n')) || input.crLfechoEnable)
                    {
                        serial.write(ch);       // echo received character
                    }
                }
                if ((ch == terminators[0]) || (ch == terminators[1]))
                {
                    if ((ch != '\r') && (ch != '\n'))
                    {
                        input.g_cCmdBuf[input.index++] = ch;    // include termination character
                    }
                    // end-of-command
                    break;
                }
                else if (ch == CHAR_BS)
                {
                    if (input.index)
                    {
                        --input.index;
                    }
                }
                else if (ch == '\n')
                {
                    // ignore LF
                }
                else
                {
                    input.g_cCmdBuf[input.index++] = ch;
                }
            }
            else
            {
                return 0;       // no command to process yet
            }
        }
        input.g_cCmdBuf[input.index] = '\0';

        if (strlen(input.g_cCmdBuf) > 0)
        {
            if (((ch == '\r') || (ch == '\n')) && input.crLfcmdEnable)
            {
                serial.println();
            }
            //
            // Pass the line from the user to the command processor.
            // It will be parsed and valid commands executed.
            //
            nStatus = CmdLineProcess(input.g_cCmdBuf);
            if (errorFunc != NULL)
            {
                errorFunc(nStatus);
            }
            else    // internal commands error handling
            {
                switch (nStatus)
                {
                    // Handle the case of bad command.
                    case CMDLINE_BAD_CMD:
                        serial.println(F("Bad command!"));
                        break;

                    // Handle the case of too many arguments.
                    case CMDLINE_TOO_MANY_ARGS:
                        serial.println(F("Too many arguments for command processor!"));
                        break;

                    // Handle the case of too few arguments.
                    case CMDLINE_TOO_FEW_ARGS:
                        serial.println(F("Not enough arguments for command processor!"));
                        break;

                    // Handle the case of invalid argument.
                    case CMDLINE_INVALID_ARG:
                        serial.println(F("Invalid argument for command processor!"));
                        break;

                    // Otherwise the command was executed.  Print the error
                    // code if one was returned.
                    default:
                        if (nStatus != 0)
                        {
                            serial.print(F("Command returned error code: "));
                            serial.println(nStatus);
                        }
                        break;
                }
            }
        }
        input.index = 0;
        return 1;       // command processed
    }
}

/*
 * NAME:
 *  int8_t CmdLineProcess(char * pcCmdLine)
 *
 * PARAMETERS:
 *  char * pcCmdLine = string that contains a command line
 *
 * WHAT:
 *  Processes a command line string into arguments and executes the command.
 *
 *  This function takes the supplied command line string and breaks it up
 *  into individual arguments.  The first argument is treated as a command and
 *  is searched for in the command table.  If the command is found, then the
 *  command function is called and all of the command line arguments are passed
 *  in the normal argc, argv form.
 *
 *  The command table is contained in a menu array named "g_sCmdTable" which
 *  must be provided by the application.
 *
 * RETURN VALUES:
 *  int8_t = CMDLINE_BAD_CMD if the command is not found,
 *         = CMDLINE_TOO_MANY_ARGS if there are more arguments than can be parsed.
 *           Otherwise it returns the code that was returned by the command function.
 *
 * SPECIAL CONSIDERATIONS:
 *  If the command is not found in the menu array and a default handler has been
 *  set up (see SetDefaultHandler()), the default handler will be called to handle
 *  the unknown command.
 */
int8_t CommandLine::CmdLineProcess(char * pcCmdLine)
{
    char * pcChar;
    int8_t argc;
    uint8_t bFindArg = 1;
    pfnCmdLine menuFunc;
    tCmdLineEntry * pCmdEntry;
#ifdef ESP8266
    PGM_P * pgmp;
    pfnCmdLine * pfn;
#endif

    //
    // Initialize the argument counter, and point to the beginning of the
    // command line string.
    //
    argc = 0;
    pcChar = pcCmdLine;

    //
    // Advance through the command line until a zero character (end of command line) is found.
    //
    while (*pcChar)
    {
        //
        // If there is the delimiter, then replace it with a zero, and set the flag
        // to search for the next argument.
        //
        if (*pcChar == delimiter)
        {
            *pcChar = '\0';
            bFindArg = 1;
        }
        //
        // Otherwise it is not a delimiter, so it must be a character that is part
        // of an argument.
        //
        else
        {
            //
            // If bFindArg is set, then that means we are looking for the start
            // of the next argument.
            //
            if (bFindArg)
            {
                //
                // As long as the maximum number of arguments has not been
                // reached, then save the pointer to the start of this new arg
                // in the argv array, and increment the count of args, argc.
                //
                if (argc < CMDLINE_MAX_ARGS)
                {
                    argv[argc] = pcChar;
                    argc++;
                    bFindArg = 0;
                }
                //
                // The maximum number of arguments has been reached so return
                // the error.
                //
                else
                {
                    return CMDLINE_TOO_MANY_ARGS;
                }
            }
        }

        //
        // Advance to the next character in the command line.
        //
        pcChar++;
    }

    //
    // If one or more arguments was found, then process the command.
    //
    if (argc)
    {
        //
        // Start at the beginning of the command table, to look for a matching
        // command.
        //
        pCmdEntry = (tCmdLineEntry *)&g_sCmdTable[0];

        //
        // Search through the command table until a null command string is
        // found, which marks the end of the table.
        //
        while (1)
        {
#ifdef ESP8266
            pgmp = &(pCmdEntry->pcCmd);    // prevents dereferencing type-punned pointer warning
            if (pgm_read_dword(pgmp) == 0)
#elif defined(ESP32) || ((defined(TEENSYDUINO) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)) && !defined(__AVR__))
            if (pCmdEntry->pcCmd == 0)
#else
            if (pgm_read_word(&pCmdEntry->pcCmd) == 0)
#endif
            {
                break;
            }
            //
            // If this command entry command string matches argv[0], then call
            // the function for this command, passing the command line
            // arguments.
            //
#ifdef ESP8266
            if (!strcasecmp_P(argv[0], (PGM_P)pgm_read_dword(pgmp)))
#elif defined(ESP32) || ((defined(TEENSYDUINO) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)) && !defined(__AVR__))
            if (!strcasecmp(argv[0], pCmdEntry->pcCmd))
#else
            if (!strcasecmp_P(argv[0], (PGM_P)pgm_read_word(&pCmdEntry->pcCmd)))
#endif
            {
#ifdef ESP8266
                pfn = &(pCmdEntry->pfnCmd);    // prevents dereferencing type-punned pointer warning
                menuFunc = (pfnCmdLine)pgm_read_dword(pfn);
#elif defined(ESP32) || ((defined(TEENSYDUINO) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)) && !defined(__AVR__))
                menuFunc = pCmdEntry->pfnCmd;
#else
                menuFunc = (pfnCmdLine)pgm_read_word(&pCmdEntry->pfnCmd);
#endif
                return menuFunc(argc, argv);
            }

            //
            // Not found, so advance to the next entry.
            //
            pCmdEntry++;
        }
    }

    //
    // Fall through to here means that no matching command was found, so return
    // an error or call a configured default command handler.
    //
    if (defaultFunc == NULL)
    {
        return CMDLINE_BAD_CMD;
    }
    else
    {
        return defaultFunc(argc, argv);
    }
}

/*
 * NAME:
 *  int8_t ParseParam(char * param, int32_t * retval)
 *
 * PARAMETERS:
 *  char * param = parameter string to parse
 *  int32_t * retval = place for parsed numeric value (if type DECVAL or HEXVAL)
 *
 * WHAT:
 *  Support routine that parses a parameter string into a decimal or hex
 *  numeric value or identifies it as a quoted string.
 *
 * RETURN VALUES:
 *  int8_t = type of parameter
 *            - DECVAL   = decimal numeric parameter value (1234) - value returned at *retval
 *            - HEXVAL   = hex numeric parameter value (0x12ab) - value returned at *retval
 *            - STRVAL   = string parameter value ("quoted string")
 *            - BADPARAM = bad parameter
 *
 * SPECIAL CONSIDERATIONS:
 *  The command line processor (CmdLineProcess()) does *not* keep the contents
 *  of a quoted string intact. Each word in the string (a space is considered a
 *  word delimiter - TABs are not) is placed in a separate argument! (The argument
 *  with the first word in the quoted string contains the opening quote and the
 *  argument with the last word in the quoted string contains the closing quote.)
 */
int8_t CommandLine::ParseParam(char * param, int32_t * retval)
{
    int ch;
    int32_t val = 0;
    bool neg_val = false;

    // skip leading whitespace
    ch = *param;
    while (ch != '\0')
    {
        if (isspace(ch))
        {
            ++param;
        }
        else
        {
            break;
        }
        ch = *param;
    }

    // test for string parameter
    if (param[0] == '\"')                       // starts with '"'
    {
        if (param[strlen(param) - 1] == '\"')   // ends with '"'
        {
            return STRVAL;
        }
        else
        {
            return BADPARAM;        // bad parameter
        }
    }

    // test for/convert hex parameter
    if (param[0] == '0')
    {
        if (tolower((int)param[1]) == 'x')   // starts with "0x"
        {
            param += 2;
            for (uint8_t i = 0; i < strlen(param); ++i)
            {
                ch = toupper((int)param[i]);
                if (isxdigit(ch))
                {
                    val *= 16;
                    if (isdigit(ch))
                    {
                        val += (ch - '0');
                    }
                    else
                    {
                        val += (ch - 'A') + 10;
                    }
                }
                else
                {
                    return BADPARAM;    // bad parameter
                }
            }
            *retval = val;          // return numeric value
            return HEXVAL;
        }
    }

    // test for negative parameter
    if (param[0] == '-')
    {
        neg_val = true;     // negative
        ++param;
    }

    // test for numeric parameter
    else if (!isdigit((int)param[0]))
    {
        return BADPARAM;        // bad parameter
    }

    // test for/convert decimal parameter
    for (uint8_t i = 0; i < strlen(param); ++i)
    {
        ch = (int)param[i];
        if (isdigit(ch))
        {
            val *= 10;
            val += (ch - '0');
        }
        else
        {
            return BADPARAM;    // bad parameter
        }
    }
    if (neg_val)
    {
        val = 0 - val;
    }
    *retval = val;          // return numeric value
    return DECVAL;
}

/*
 * NAME:
 *  void Echo(bool _echoEnable)
 *
 * PARAMETERS:
 *  bool _echoEnable = a flag that is used to enable/disable echo of incoming characters
 *                     (true = enable echo, false = disable echo)
 *
 * WHAT:
 *  Enables/disables echo of incoming characters.
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  CR/LF will not be echoed if Echo() is enabled and CrLfEcho() is disabled.
 */
void CommandLine::Echo(bool _echoEnable)
{
    input.echoEnable = _echoEnable;
}

/*
 * NAME:
 *  void CrLfEcho(bool _echoEnable)
 *
 * PARAMETERS:
 *  bool _echoEnable = a flag that is used to enable/disable echo of incoming CR/LF characters
 *                     (true = enable echo, false = disable echo)
 *
 * WHAT:
 *  Enables/disables echo of incoming CR/LF characters.
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  This has no effect if Echo() is disabled.
 */
void CommandLine::CrLfEcho(bool _echoEnable)
{
    input.crLfechoEnable = _echoEnable;
}

/*
 * NAME:
 *  void CrLfCommand(bool _crLfcmdEnable)
 *
 * PARAMETERS:
 *  bool _crLfcmdEnable = a flag that is used to enable/disable sending CR/LF characters
 *                        before processing a command
 *                        (true = enable CR/LF response, false = disable CR/LF response)
 *
 * WHAT:
 *  Enables/disables sending CR/LF characters before processing command.
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
void CommandLine::CrLfCommand(bool _crLfcmdEnable)
{
    input.crLfcmdEnable = _crLfcmdEnable;
}

/*
 * NAME:
 *  void Delimiter(char _delimiter)
 *
 * PARAMETERS:
 *  char _delimiter = the delimiter (or parameter separator) character
 *
 * WHAT:
 *  Sets the parameter separator character to use (if other than the default space ' ').
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
void CommandLine::Delimiter(char _delimiter)
{
    delimiter = _delimiter;
}

/*
 * NAME:
 *  void Terminators(char * _terminators)
 *
 * PARAMETERS:
 *  char * _terminators = the command line terminator character(s) to use (2 maximum supported)
 *
 * WHAT:
 *  Sets the parameter separator character(s) to use (if other than the default '\r').
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
void CommandLine::Terminators(char * _terminators)
{
    strncpy(terminators, _terminators, CMDLINE_MAX_TERMINATORS);
}

/*
 * NAME:
 *  void SetDefaultHandler(pfnCmdLine function)
 *
 * PARAMETERS:
 *  pfnCmdLine function = the handler function for unknown commands
 *
 * WHAT:
 *  Sets the handler function to use for unknown commands (default is none).
 *  (i.e. commands not in command table)
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
void CommandLine::SetDefaultHandler(pfnCmdLine function)
{
    defaultFunc = function;
}

/*
 * NAME:
 *  void SetCustomErrorHandler(pfnCustomErrs function)
 *
 * PARAMETERS:
 *  pfnCustomErrs function = the custom handler function for commands errors
 *
 * WHAT:
 *  Sets the custom handler function to use for commands errors (default is internal).
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
void CommandLine::SetCustomErrorHandler(pfnCustomErrs function)
{
    errorFunc = function;
}

/*
 * NAME:
 *  void ShowCommands(bool help_info_disable)
 *
 * PARAMETERS:
 *  bool help_info_disable = optional flag to disable (if true) showing help information
 *                           (default = false)
 *
 * WHAT:
 *  Shows the menu commands.
 *
 * RETURN VALUES:
 *  None.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
void CommandLine::ShowCommands(bool help_info_disable)
{
    tCmdLineEntry * pEntry;
#ifdef ESP8266
    PGM_P * pgmp;
#endif

    // Point at the beginning of the command table.
    pEntry = (tCmdLineEntry *)&g_sCmdTable[0];

    //
    // Enter a loop to read each entry from the command table.  The
    // end of the table has been reached when the command name is NULL.
    //
    while (1)
    {
#ifdef ESP8266
        pgmp = &(pEntry->pcCmd);    // prevents dereferencing type-punned pointer warning
        if (pgm_read_dword(pgmp) == 0)
#elif defined(ESP32) || ((defined(TEENSYDUINO) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)) && !defined(__AVR__))
        if (pEntry->pcCmd == 0)
#else
        if (pgm_read_word(&pEntry->pcCmd) == 0)
#endif
        {
            break;
        }
        // Print the command name and the brief description.
        // See: http://forum.arduino.cc/index.php?topic=392256.0
#ifdef ESP8266
        serial.print((const __FlashStringHelper *)pgm_read_dword(pgmp));
#elif defined(ESP32) || ((defined(TEENSYDUINO) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)) && !defined(__AVR__))
        serial.print(pEntry->pcCmd);
#else
        serial.print((const __FlashStringHelper *)pgm_read_word(&(pEntry->pcCmd)));
#endif
        if (!help_info_disable)
        {
#ifdef ESP8266
            pgmp = &(pEntry->pcHelp);   // prevents dereferencing type-punned pointer warning
            serial.println((const __FlashStringHelper *)pgm_read_dword(pgmp));
#elif defined(ESP32) || ((defined(TEENSYDUINO) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_MBED_RP2040)) && !defined(__AVR__))
            serial.println(pEntry->pcHelp);
#else
            serial.println((const __FlashStringHelper *)pgm_read_word(&(pEntry->pcHelp)));
#endif
        }
        else
        {
            serial.println();
        }

        // Advance to the next entry in the table.
        ++pEntry;
    }
}

