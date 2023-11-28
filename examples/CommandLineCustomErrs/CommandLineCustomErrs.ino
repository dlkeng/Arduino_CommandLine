/*
 * NAME: CommandLineCustomErrs.ino
 *
 * WHAT:
 *  An Arduino serial command line I/F custom error handling test program.
 *
 *  It demonstrates use of CommandLine library:
 *   - command line menus (stored and acted on in Flash)
 *   - parsing command line parameters
 *   - custom error handler for processing errors
 *
 *  Can be used as a template for a larger application.
 *
 * SPECIAL CONSIDERATIONS:
 *  None.
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

#include <CommandLine.h>

#define TITLE_MSG           "Arduino Command Line Custom Error Handler Testing"

#define LED_PIN     LED_BUILTIN         // the Arduino LED pin

#define LED_ON      HIGH
#define LED_OFF     LOW
#define LED_HB      -1

#define TIMER_EXPIRED(start, interval)  ((millis() - start) >= interval)
#define HEARTBEAT_OFF_INTERVAL  950     // mS
#define HEARTBEAT_ON_INTERVAL   50      // mS

int8_t LedState = LED_HB;

CommandLine CmdLine(Serial);    // setup CommandLine to use standard Arduino Serial and echo on

bool VerboseErrsEnabled = true;

void setup()
{
    // init Arduino LED
    pinMode(LED_PIN, OUTPUT);
    LED_off();

    // setup serial port
    Serial.begin(115200);

    // generate the sign-on banner
    Serial.println(F(CLS_HOME));
    Serial.println(F("CommandLineTest.ino"));
    Serial.println(F(TITLE_MSG));
   
    // set a custom error handler
    CmdLine.SetCustomErrorHandler(ErrHandler);
   
    delay(50);
}

void loop()
{
    static bool new_prompt = true;

    if (new_prompt)
    {
        new_prompt = false;
        //
        // Print a prompt to the console.
        //
        Serial.println();
        Serial.print(F("-> "));
    }

    new_prompt = CmdLine.DoCmdLine();

    // do other stuff here

    // do Heartbeat
    DoHeartbeat();
}

// Custom command error handler for commands errors
void ErrHandler(int8_t err_code)
{
    if (err_code != 0)      // has an error code
    {
        Serial.print("ERR:");
        Serial.print(err_code);
        if (!VerboseErrsEnabled)
        {
            Serial.println();
        }
        else    // verbose error reporting
        {
            Serial.print("::");
            switch (err_code)
            {
                // Handle the case of bad command.
                case CMDLINE_BAD_CMD:
                    Serial.println(F("Bad command!"));
                    break;

                // Handle the case of too many arguments.
                case CMDLINE_TOO_MANY_ARGS:
                    Serial.println(F("Too many arguments for command processor!"));
                    break;

                // Handle the case of too few arguments.
                case CMDLINE_TOO_FEW_ARGS:
                    Serial.println(F("Not enough arguments for command processor!"));
                    break;

                // Handle the case of invalid argument.
                case CMDLINE_INVALID_ARG:
                    Serial.println(F("Invalid argument for command processor!"));
                    break;

                // Otherwise the command was executed.  Print the error
                // code if one was returned.
                default:
                    Serial.print(F("Command returned error code: "));
                    Serial.println(err_code);
                    break;
            }
        }
    }
}

// do Heartbeat
void DoHeartbeat(void)
{
    static uint32_t last_HB_tick = 0;
    static uint8_t last_HB_state = false;

    if (LedState == LED_HB)
    {
        if (last_HB_state)
        {
            if (TIMER_EXPIRED(last_HB_tick, HEARTBEAT_ON_INTERVAL))
            {
                LED_off();      // off
                last_HB_state = false;
                last_HB_tick = millis();
            }
        }
        else
        {
            if (TIMER_EXPIRED(last_HB_tick, HEARTBEAT_OFF_INTERVAL))
            {
                LED_on();       // on
                last_HB_state = true;
                last_HB_tick = millis();
            }
        }
    }
}

void LED_on(void)
{
    digitalWrite(LED_PIN, LED_ON);
}

void LED_off(void)
{
    digitalWrite(LED_PIN, LED_OFF);
}

