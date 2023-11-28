/*
 * NAME: CommandLineTest.c
 *
 * WHAT:
 *  A simple Arduino serial command line I/F test program.
 *
 *  It demonstrates use of CommandLine library:
 *   - command line menus (stored and acted on in Flash)
 *   - parsing command line parameters
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

#define TITLE_MSG           "Arduino Command Line Testing"

#define LED_PIN     LED_BUILTIN         // the Arduino LED pin

#define LED_ON      HIGH
#define LED_OFF     LOW
#define LED_HB      -1

#define TIMER_EXPIRED(start, interval)  ((millis() - start) >= interval)
#define HEARTBEAT_OFF_INTERVAL  950     // mS
#define HEARTBEAT_ON_INTERVAL   50      // mS

int8_t LedState = LED_HB;

CommandLine CmdLine(Serial);    // setup CommandLine to use standard Arduino Serial and echo on

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

//    CmdLine.Echo(false);          // disable echo of received characters (default enabled)
//    CmdLine.CrLfCommand(false);   // disable sending CR/LF characters before processing command (default enabled)
//    CmdLine.Delimiter(',');       // change delimiter from default ' ' to ',' (space to comma)

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

