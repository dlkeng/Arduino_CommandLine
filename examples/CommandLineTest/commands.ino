/*
 * NAME: commands.ino
 *
 * WHAT:
 *  Commands for serial command line I/F.
 *
 * SPECIAL CONSIDERATIONS:
 *  None
 *
 * AUTHOR:
 *  D.L. Karmann
 *
 * MODIFIED:
 *
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t Input_Value = 0;

// local function prototypes
int8_t Cmd_help(int8_t argc, char * argv[]);
int8_t Cmd_led(int8_t argc, char * argv[]);
int8_t Cmd_show(int8_t argc, char * argv[]);
int8_t Cmd_input(int8_t argc, char * argv[]);

//*****************************************************************************
//
// These are the command names and brief descriptions.
//
// To add a menu item: (to keep all items in Flash)
//   1) add the command string to the 'MenuCmd#' item
//   2) add the command help string to the 'MenuHelp#' item
//   3) add the function prototype for the command's function above
//   4) add the 'MenuCmd#', function's name, and 'MenuHelp#' to the 'g_sCmdTable[]' array
//   5) add the function for processing the command to this file
//
//*****************************************************************************

// menu items individual command name strings
const char MenuCmdHelp1[] PROGMEM = "help";
const char MenuCmdHelp2[] PROGMEM = "h";
const char MenuCmdHelp3[] PROGMEM = "?";
const char MenuCmdLed[] PROGMEM   = "led";
const char MenuCmdShow[] PROGMEM  = "show";
const char MenuCmdInput[] PROGMEM = "input";

// menu items individual command help strings
const char MenuHelp1[] PROGMEM     =    " [<cls>]          : Display list of commands (clear screen)";
const char MenuHelp2[] PROGMEM     = "                     : alias for help";
const char MenuHelpLed[] PROGMEM   =   " [<on | off | hb>] : Show/control the LED";
const char MenuHelpShow[] PROGMEM  =    " [params]         : Show command line parameters";
const char MenuHelpInput[] PROGMEM =     " [vals]          : Show/set command line numeric value";

//*****************************************************************************
//
// This is the table that holds the command names, implementing functions,
// and brief description. (Required by the 'CommandLine' command processor.)
//
//*****************************************************************************
const tCmdLineEntry g_sCmdTable[] PROGMEM =
{
    //  command     function        help info
    { MenuCmdHelp1, Cmd_help,  MenuHelp1     },
    { MenuCmdHelp2, Cmd_help,  MenuHelp2     },
    { MenuCmdHelp3, Cmd_help,  MenuHelp2     },
    { MenuCmdLed,   Cmd_led,   MenuHelpLed   },
    { MenuCmdShow,  Cmd_show,  MenuHelpShow  },
    { MenuCmdInput, Cmd_input, MenuHelpInput },
    { 0, 0, 0 }     // end of commands
};

/*
 * NAME:
 *  int8_t Cmd_led(int8_t argc, char * argv[])
 *
 * PARAMETERS:
 *  int8_t argc = number of command line arguments for the command
 *  char * argv[] = pointer to array of parameters associated with the command
 *
 * WHAT:
 *  Implements the "led" command to query status of and turn the on-board LED on or
 *  off or heartbeat.
 *
 * RETURN VALUES:
 *  int8_t = 0 = command successfully processed
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
int8_t Cmd_led(int8_t argc, char * argv[])
{
	if (argc > 2)
	{
		return CMDLINE_TOO_MANY_ARGS;
	}
	else if (argc > 1)      // has a command argument
	{
		if (strcmp_P(argv[ARG1], PSTR("on")) == 0)
		{
            LED_on();
            LedState = LED_ON;
		}
		else if (strcmp_P(argv[ARG1], PSTR("off")) == 0)
		{
            LED_off();
            LedState = LED_OFF;
		}
		else if (strcmp_P(argv[ARG1], PSTR("hb")) == 0)
		{
            LedState = LED_HB;
		}
		else    // unknown/invalid argument
		{
			return CMDLINE_BAD_CMD;
		}
	}

    Serial.print(F("On-Board LED: "));
    switch (LedState)
    {
        case LED_ON:
            Serial.println(F("on"));
            break;
        case LED_OFF:
            Serial.println(F("off"));
            break;
        case LED_HB:
            Serial.println(F("HB"));
            break;
    }

    // Return success.
    return 0;
}

/*
 * NAME:
 *  int8_t Cmd_show(int8_t argc, char * argv[])
 *
 * PARAMETERS:
 *  int8_t argc = number of command line arguments for the command
 *  char * argv[] = pointer to array of parameters associated with the command
 *
 * WHAT:
 *  Implements the "show" command to to show example command line items.
 *
 * RETURN VALUES:
 *  int8_t = 0 = command successfully processed
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
int8_t Cmd_show(int8_t argc, char * argv[])
{
	if (argc > CMDLINE_MAX_ARGS)
	{
		return CMDLINE_TOO_MANY_ARGS;
	}
	else
	{
        Serial.println(F("Command Line: "));
        for (uint8_t i = 0; i < argc; ++i)
        {
            // show the parameter
            switch (i)
            {
                case CMD:
                    Serial.print(F(" Cmd: "));
                    break;
                case ARG1:
                    Serial.print(F(" Arg1: "));
                    break;
                case ARG2:
                    Serial.print(F(" Arg2: "));
                    break;
                case ARG3:
                    Serial.print(F(" Arg3: "));
                    break;
                case ARG4:
                    Serial.print(F(" Arg4: "));
                    break;
                case ARG5:
                    Serial.print(F(" Arg5: "));
                    break;
                case ARG6:
                    Serial.print(F(" Arg6: "));
                    break;
                case ARG7:
                    Serial.print(F(" Arg7: "));
                    break;
                case ARG8:
                    Serial.print(F(" Arg8: "));
                    break;
                case ARG9:
                    Serial.print(F(" Arg9: "));
                    break;
            }
            Serial.println(argv[i]);
        }
    }

    // Return success.
    return 0;
}

/*
 * NAME:
 *  int8_t Cmd_input(int8_t argc, char * argv[])
 *
 * PARAMETERS:
 *  int8_t argc = number of command line arguments for the command
 *  char * argv[] = pointer to array of parameters associated with the command
 *
 * WHAT:
 *  Implements the "input" command to show/set the example numeric input values.
 *
 * RETURN VALUES:
 *  int8_t = 0 = command successfully processed
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
int8_t Cmd_input(int8_t argc, char * argv[])
{
    uint32_t val;
    int8_t paramtype;

	if (argc > CMDLINE_MAX_ARGS)
	{
		return CMDLINE_TOO_MANY_ARGS;
	}
	else if (argc > 1)
	{
        // get the input value
        paramtype = CmdLine.ParseParam(argv[ARG1], &val);
        if ((paramtype == BADPARAM) || (paramtype == STRVAL) ||
            (val < 1) || (val > 1000000))
        {
            return CMDLINE_INVALID_ARG;
        }
        if (paramtype == DECVAL)
        {
            Serial.print(F("Dec:"));
        }
        else if (paramtype == HEXVAL)
        {
            Serial.print(F("Hex:"));
        }
        Input_Value = val;
    }

    Serial.print(F("Input Value: "));
    Serial.println(Input_Value);

    // Return success.
    return 0;
}

/*
 * NAME:
 *  int8_t Cmd_help(int8_t argc, char * argv[])
 *
 * PARAMETERS:
 *  int8_t argc = number of command line arguments for the command
 *  char * argv[] = pointer to array of parameters associated with the command
 *
 * WHAT:
 *  Implements the "help" command to display a simple list of the available
 *  commands with a brief description of each command.
 *
 *  One optional parameter supported.
 *   <cls> = clears the output screen using ANSI escape sequence
 *
 * RETURN VALUES:
 *  int8_t = 0 = command successfully processed
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
 */
int8_t Cmd_help(int8_t argc, char * argv[])
{
    if (argc > 1)
    {
        if (strcmp_P(argv[ARG1], PSTR("cls")) == 0)
        {
            Serial.println(F(CLS_HOME));
            return 0;
        }
    }

    // Print some header text.
    Serial.println("");
    Serial.println(TITLE_MSG);
    Serial.println(F("Available commands"));
    Serial.println(F("------------------"));

    CmdLine.ShowCommands();     // show commands menu with help information

    // Return success.
    return 0;
}

