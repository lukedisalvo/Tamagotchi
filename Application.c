/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* HAL and Application includes */
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>



/**
 * The main entry point of your project. The main function should immediately
 * stop the Watchdog timer, call the Application constructor, and then
 * repeatedly call the main super-loop function. The Application constructor
 * should be responsible for initializing all hardware components as well as all
 * other finite state machines you choose to use in this project.
 *
 * THIS FUNCTION IS ALREADY COMPLETE. Unless you want to temporarily experiment
 * with some behavior of a code snippet you may have, we DO NOT RECOMMEND
 * modifying this function in any way.
 */
int main(void)
{
    // Stop Watchdog Timer - THIS SHOULD ALWAYS BE THE FIRST LINE OF YOUR MAIN
    WDT_A_holdTimer();

    // Initialize the system clock and background hardware timer, used to enable
    // software timers to time their measurements properly.
    InitSystemTiming();

    // Initialize the main Application object and HAL object
    HAL hal = HAL_construct();
    Application app = Application_construct();

    // Main super-loop! In a polling architecture, this function should call
    // your main FSM function over and over.
    while (true)
    {
        Application_loop(&app, &hal);
        HAL_refresh(&hal);

    }

}

/**
 * A helper function which increments a value with a maximum. If incrementing
 * the number causes the value to hit its maximum, the number wraps around
 * to 0.
 */
uint32_t CircularIncrement(uint32_t value, uint32_t maximum)
{
    return (value + 1) % maximum;
}

/**
 * The main constructor for your application. This function should initialize
 * each of the FSMs which implement the application logic of your project.
 *
 * @return a completely initialized Application object
 */

Application Application_construct()
{
    Application app;

    // Initialize local application state variables here!
    app.baudChoice = BAUD_9600;
    app.firstCall = true;

    app.gfx = GFX_construct(GRAPHICS_COLOR_WHITE, GRAPHICS_COLOR_WHITE);

    app.year = SWTimer_construct(3000);

    SWTimer_start(&app.year);

    return app;
}

/**
 * The main super-loop function of the application. We place this inside of a
 * single infinite loop in main. In this way, we can model a polling system of
 * FSMs. Every cycle of this loop function, we poll each of the FSMs one time,
 * followed by refreshing all inputs to the system through a convenient
 * [HAL_refresh()] call.
 *
 * @param app:  A pointer to the main Application object.
 * @param hal:  A pointer to the main HAL object
 */
void Application_loop(Application* app, HAL* hal)
{
   //Non-Blocking test. Pressing LauchpadS1 turns launchpad LED1 on and releasing it turns it off
    LED_turnOff(&hal->launchpadLED1);
    if (Button_isPressed(&hal->launchpadS1)) {
        LED_turnOn(&hal->launchpadLED1);
    }


    static int Age = 0;
    char age1[10];
    sprintf(age1, "Age: %d", Age);

    int baudRate = app->baudChoice;
    char baud[10];
    sprintf(baud, "BR: %d", baudRate);

    char* happy1 = app->Happy;
    char* energy1 = app->Energy;


    //draws the circle in first position
    Graphics_setForegroundColor(&app->gfx.context, 230);
    Graphics_fillCircle(&app->gfx.context, 30, 40, 15);
    //draws the circle in second position
    Graphics_fillCircle(&app->gfx.context, 64, 40, 15);
    //draws the circle in third position
    Graphics_fillCircle(&app->gfx.context, 100, 40, 15);
    //draws the circle in fourth position
    Graphics_fillCircle(&app->gfx.context, 30, 85, 15);
    //draws the circle in fifth position
    Graphics_fillCircle(&app->gfx.context, 64, 85, 15);
    //draws the circle in sixth position
    Graphics_fillCircle(&app->gfx.context, 100, 85, 15);

    Graphics_setForegroundColor(&app->gfx.context, 0);
    Graphics_drawLineH(&app->gfx.context, 10, 120, 20);
    Graphics_drawLineV(&app->gfx.context, 10, 20, 105);
    Graphics_drawLineV(&app->gfx.context, 120, 20, 105);
    Graphics_drawLineH(&app->gfx.context, 10, 120, 105);
    Graphics_drawString(&app->gfx.context, (int8_t*) baud, -1, 90, 0, true);
    Graphics_drawString(&app->gfx.context, (int8_t*) age1, -1, 0, 0, true);
    Graphics_drawString(&app->gfx.context, (int8_t*) happy1, -1, 0, 110, true);
    Graphics_drawString(&app->gfx.context, (int8_t*) energy1, -1, 0, 120, true);
    Application_updateHappiness(app, hal);
    Application_updateEnergy(app, hal);
    app->Feed = false;
    app->Move = false;





    // Update communications if either this is the first time the application is
    // run or if Boosterpack S1 is pressed.
    if (Button_isTapped(&hal->boosterpackS2) || app->firstCall) {
        Application_updateCommunications(app, hal);
    }

    // Interpret a new character if one is received.
    if (UART_hasChar(&hal->uart))
    {
        Application_interpretIncomingChar(app, hal);
    }

    //This creates a static variable for the the age and creates a char called buffer to store the string of age
    //in the buffer. I print the buffer and use that in the drawString function in order to pass a variable.
    //If the timer expires, then the timer is reset and the age is increased and displayed on the LCD.
    if(SWTimer_expired(&app->year))
    {
        LED_toggle(&hal->launchpadLED2Red);
        SWTimer_start(&app->year);
        Age++;
    }

    //Lines 158-162 take the value in the update_Communications function and see the value for the Baudrate
    //The Baudrate is a number from 0-3.
    //By creating an integrer that points to the baudchoice, we can use the same fucntionality as presenting
    //the age and by pressing Button 2, the baudrate should change based on the update_Communications

    Application_Death(app, hal);



}

void Application_updateHappiness(Application* app, HAL* hal)
{
   static int Happiness = 5;
   app->notHappy = false;
    if(SWTimer_expired(&app->year))
    {
        if(Happiness == 0)
        {
            Happiness = Happiness;
        }
        else
        {
            Happiness--;
        }
    }

    if (app->Move == true)
    {
        if (Happiness == 5)
        {
            Happiness = Happiness;
        }
        else
        {
            Happiness++;

        }
    }


    switch (Happiness)
    {
        case 5:
            app->Happy = "Happy: *****";
            break;

        case 4:
            app->Happy = "Happy: **** ";
            break;
        case 3:
            app->Happy = "Happy: ***  ";
            break;
        case 2:
            app->Happy = "Happy: **   ";
            break;
        case 1:
            app->Happy = "Happy: *    ";
            break;
        case 0:
            app->Happy = "Happy:      ";
            app->notHappy = true;
                break;
        default:
                break;
    }
}

void Application_updateEnergy(Application* app, HAL* hal)
{
    static int Energy = 5;
    app->notEnergetic = false;

    if(SWTimer_expired(&app->year) || app->Move == true)
    {
        if(Energy == 0)
        {
            Energy = Energy;
        }
        else
        {
            Energy--;
        }
    }

    if (app->Feed == true)
    {
        if (Energy == 5)
        {
            Energy = Energy;
        }
        else
        {
            Energy++;
        }
    }


    switch (Energy)
    {
        case 5:
            app->Energy = "Energy: *****";
            break;

        case 4:
            app->Energy = "Energy: **** ";
            break;
        case 3:
            app->Energy = "Energy: ***  ";
            break;
        case 2:
            app->Energy = "Energy: **   ";
            break;
        case 1:
            app->Energy = "Energy: *    ";
            break;
        case 0:
            app->Energy = "Energy:      ";
            app->notEnergetic = true;
            break;
        default:
            break;
    }
}




void Application_Death(Application* app, HAL* hal)
{

    if (app->notEnergetic == true && app->notHappy == true)
    {
        Graphics_clearDisplay(&app->gfx.context);
        Graphics_drawString(&app->gfx.context, "You died", -1, 40, 64, true);
        Graphics_drawString(&app->gfx.context, "Game Over", -1, 0, 0, true);
        exit(0);
    }
}
/**
 * Updates which LEDs are lit and what baud rate the UART module communicates
 * with, based on what the application's baud choice is at the time this
 * function is called.
 *
 * @param app:  A pointer to the main Application object.
 * @param hal:  A pointer to the main HAL object
 */
void Application_updateCommunications(Application* app, HAL* hal)
{
    // When this application first loops, the proper LEDs aren't lit. The
    // firstCall flag is used to ensure that the
    if (app->firstCall) {
        app->firstCall = false;
    }

    // When Boosterpack S1 is tapped, circularly increment which baud rate is used.
    if (Button_isTapped(&hal->boosterpackS2))
    {
        uint32_t newBaudNumber = CircularIncrement((uint32_t) app->baudChoice, NUM_BAUD_CHOICES);
        app->baudChoice = (UART_Baudrate) newBaudNumber;
    }

    // Update the baud rate according to the one set above.
    UART_updateBaud(&hal->uart, app->baudChoice);

    // Based on the new application choice, turn on the correct LED.
    // To make your life easier, we recommend turning off all LEDs before
    // selectively turning back on only the LEDs that need to be relit.
    // -------------------------------------------------------------------------
    LED_turnOff(&hal->boosterpackRed);
    LED_turnOff(&hal->boosterpackGreen);
    LED_turnOff(&hal->boosterpackBlue);

    // TODO: Turn on all appropriate LEDs according to the tasks below.
    switch (app->baudChoice)
    {
        // When the baud rate is 9600, turn on Boosterpack LED Red
        case BAUD_9600:
            LED_turnOn(&hal->boosterpackRed);
            break;

        // TODO: When the baud rate is 19200, turn on Boosterpack LED Green
        case BAUD_19200:
            LED_turnOn(&hal->boosterpackGreen);
            break;

        // TODO: When the baud rate is 38400, turn on Boosterpack LED Blue
        case BAUD_38400:
            LED_turnOn(&hal->boosterpackBlue);
            break;

        // TODO: When the baud rate is 57600, turn on all Boosterpack LEDs (illuminates white)
        case BAUD_57600:
            LED_turnOn(&hal->boosterpackRed);
            LED_turnOn(&hal->boosterpackGreen);
            LED_turnOn(&hal->boosterpackBlue);
            break;

        // In the default case, this program will do nothing.
        default:
            break;
    }
}

/**
 * Interprets a character which was incoming and sends an interpreted character
 * back if the system USB UART module can accept a new character.
 *
 * @param app:  A pointer to the main Application object.
 * @param hal:  A pointer to the main HAL object
 */
void Application_interpretIncomingChar(Application* app, HAL* hal)
{
    // The character received from your serial terminal
    char rxChar = UART_getChar(&hal->uart);
    char txChar = 'O';


    // The character to return back to sender. By default, we assume the letter
    // to send back is an 'O' (assume the character is an "other" character)

    // Numbers - if the character entered was a number, transfer back an 'N'
    if (rxChar >= '0' && rxChar <= '9')
    {
        txChar = 'N';
    }

    // Letters - if the character entered was a letter, transfer back an 'L'
    if ((rxChar >= 'a' && rxChar <= 'z') || (rxChar >= 'A' && rxChar <= 'Z'))
    {
        // Letters - if the character entered was a letter, transfer back an 'L'
        if (rxChar == 'f' || rxChar == 'F')
        {
            txChar = 'f';
        }
        else if (rxChar == 'w'|| rxChar == 'W')
        {
            txChar = 'w';
        }
        else if (rxChar == 'a'|| rxChar == 'A')
        {
            txChar = 'a';
        }
        else if (rxChar == 's'|| rxChar == 'S')
        {
            txChar = 's';
        }
        else if (rxChar == 'd' || rxChar == 'D')
        {
            txChar = 'd';
        }
        else
            txChar = 'L';
    }

    if (UART_canSend(&hal->uart))
    {
        UART_putChar(&hal->uart, txChar);
    }
    app->Feed = false;
    app->Move = false;
    static int num_Moves = 0;
    if (txChar == 'f')
    {
        app->Feed = true;
    }
    else
    {
        app->Feed = false;
    }
    if (txChar == 'w' || txChar == 'a' || txChar == 's' || txChar == 'd')
    {
        num_Moves++;
        if(num_Moves%4 == 0)
        {
            app->Move = true;
        }
        else
        {
            app->Move = false;
        }
    }

    // Only send a character if the UART module can send it back

}

