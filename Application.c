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
// external global variables for the extra credit to draw the Tamagotchi as images other than circles
extern tImage GrimReaper8BPP_UNCOMP;
extern tImage Piplup4BPP_UNCOMP;
extern tImage Prinplup8BPP_UNCOMP;
extern tImage  Empoleon8BPP_UNCOMP;

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

    //Pre-setting variables used in code
    app.Eng = 5; // Energy level starts at 5
    app.Hap = 5; //Happiness level starts at 5
    app.x = 20; // x-coordinate starts at 20
    app.y = 30; //y-coordinate starts at 30
    app.num_Moves = 0; //Number of moves starts at 0
    app.Age = 0; //Age starts at 0
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
    if (Button_isPressed(&hal->launchpadS1))
    {
        LED_turnOn(&hal->launchpadLED1);
    }
    //If the tamagotchi is not in the dead state, then continue with this loop
    if ((app->notEnergetic ==  0 && app->notHappy == 0) || (app->notEnergetic == 0   && app->notHappy == 1) || (app->notEnergetic == 1 && app->notHappy == 0))
    {
        //Creates a char called buffer to store the string of Age in the buffer.
        //I print the buffer and use that in the drawString function in order to pass a variable.
        //If the timer expires, then the timer is reset and the age is increased and displayed on the LCD.
        char age1[10];
        sprintf(age1, "Age: %d", app->Age);
        //Creates a char called baud to store the string of ABR in the buffer.
        int baudRate = app->baudChoice;
        char baud[10];
        sprintf(baud, "BR: %d", baudRate);
        //Creates char pointers that point to energy and happy levels in another function so that they can be displayed
        //on the LCD display
        char* happy1 = app->Happy;
        char* energy1 = app->Energy;

        //Creates the main display of the screen
        Graphics_setForegroundColor(&app->gfx.context, 0);
        Graphics_drawLineH(&app->gfx.context, 10, 120, 20);
        Graphics_drawLineV(&app->gfx.context, 10, 20, 105);
        Graphics_drawLineV(&app->gfx.context, 120, 20, 105);
        Graphics_drawLineH(&app->gfx.context, 10, 120, 105);
        Graphics_drawString(&app->gfx.context, (int8_t*) baud, -1, 90, 0, true);
        Graphics_drawString(&app->gfx.context, (int8_t*) age1, -1, 0, 0, true);
        Graphics_drawString(&app->gfx.context, (int8_t*) happy1, -1, 0, 110, true);
        Graphics_drawString(&app->gfx.context, (int8_t*) energy1, -1, 0, 120, true);
        //Updates the happiness level
        Application_updateHappiness(app, hal);
        //Updates the energy level
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
            //Application_Move(app, hal);
        }


        //If the timer expires, then the age will increase
        if(SWTimer_expired(&app->year))
        {
            //LED_toggle(&hal->launchpadLED2Red);
            SWTimer_start(&app->year);
            app->Age++;
        }
        //taking the age, the tamagotchi is updated based on age, happiness and energy levels
        Application_Updateaging(app, hal);

        //if the tamagotchi is in the dead state, clear all graphics and go to the else statement
        if (app->notEnergetic == true && app->notHappy == true)
        {
            Graphics_clearDisplay(&app->gfx.context);
        }
    }
    //Displays the death screen
    else
    {
        Application_Death(app, hal);
    }


}
//Application_updateHappiness decreases the happiness level every year that passes.
//If there are 4 valid movements then the happiness level us increased.
//Takes notHappy as an in from 0-5 and based on its value updates the "Happy" char
//to be printed on the LCD display in the main loop
void Application_updateHappiness(Application* app, HAL* hal)
{
    app->notHappy = 0;
    if(SWTimer_expired(&app->year))
    {
        if(app->Hap == 0)
        {
            app->Hap = app->Hap;
        }
        else
        {
            app->Hap--;
        }
    }

    if (app->Move == true)
    {
        if (app->Hap == 5)
        {
            app->Hap = app->Hap;
        }
        else
        {
            app->Hap++;

        }
    }


    switch (app->Hap)
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
            app->notHappy = 1;
                break;
        default:
                break;
    }
}
//Application_updateEnergy decreases the energy level when one year expires.
//Also, when there is 4 valid movements, decreases the energy level by 1.
//Updates an int with values 0-5 and based on those values updates a char
//that will print on the LCD display in the main loop
void Application_updateEnergy(Application* app, HAL* hal)
{
    app->notEnergetic = 0;

    if(SWTimer_expired(&app->year) || app->Move == true)
    {
        if(app->Eng == 0)
        {
            app->Eng = app->Eng;
        }
        else
        {
            app->Eng--;
        }
    }

    if (app->Feed == true)
    {
        if (app->Eng == 5)
        {
            app->Eng = app->Eng;
        }
        else
        {
            app->Eng++;
        }
    }


    switch (app->Eng)
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
            app->notEnergetic = 1;
            break;
        default:
            break;
    }
}



//Application_Death writes the death images when the death state is true
void Application_Death(Application* app, HAL* hal)
{
    Graphics_drawImage(&app->gfx.context, &GrimReaper8BPP_UNCOMP, 25, 25);
    Graphics_setForegroundColor(&app->gfx.context, 0);
    Graphics_drawString(&app->gfx.context, "Your time has come", -1, 10, 120, true);
    Graphics_drawString(&app->gfx.context, "Game Over", -1, 0, 0, true);
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
    if (app->firstCall)
    {
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
            //LED_turnOn(&hal->boosterpackGreen);
            break;

        // TODO: When the baud rate is 38400, turn on Boosterpack LED Blue
        case BAUD_38400:
           // LED_turnOn(&hal->boosterpackBlue);
            break;

        // TODO: When the baud rate is 57600, turn on all Boosterpack LEDs (illuminates white)
        case BAUD_57600:
           //LED_turnOn(&hal->boosterpackRed);
           //LED_turnOn(&hal->boosterpackGreen);
           //LED_turnOn(&hal->boosterpackBlue);
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
    app->instruction = txChar;
    //if the character is 'f' then feed is true, which will increase energy in Application_UpdateEnergy function
    if (app->instruction == 'f')
    {
        app->Feed = true;
    }
    else
    {
        app->Feed = false;
    }
    //if there are 4 valid movements, make the move boolean true which will increase Happiness in the
    //Application_UpdateHappiness function
    if(app->num_Moves%4 == 0)
    {
        app->Move = true;
    }
    else
    {
        app->Move = false;
    }




}

//Application_Updateaging take the int age and tell what tamagotchi to display.
//egg state is age 0
//child state is age 1-3
//adult stage is > 4
//the tamagotchi will only grow to the adult stage if the happiness and energy levels are greater than 4 no
//matter what the age is as long as it is > 4.
void Application_Updateaging(Application* app, HAL* hal)
{

    static int adult = 0;
    if (app->Eng >= 4 && app->Hap >= 4 && app->Age >= 4)
    {
        adult = 1;
    }
    if (app->Age == 0)
    {
        //Graphics_setForegroundColor(&app->gfx.context, 0);
        Application_Move(app, hal);
    }
    if (app->Age >= 1 && app->Age <= 3)
    {
        //Graphics_setForegroundColor(&app->gfx.context, 8190976);
        Application_Move2(app, hal);
    }
    if (app->Age >= 4 && adult == 1)
    {
        //Graphics_setForegroundColor(&app->gfx.context, 49151);
        Application_Move3(app, hal);
    }
    if (app->Age >= 4 && adult == 0)
    {
        //Graphics_setForegroundColor(&app->gfx.context, 8190976);
        Application_Move2(app, hal);
    }


}

//Application_Move has two ints x and y to tell the position of the tamagotchi.
//if WASD is pressed then depending on where the tamagotchi is, the tamagotchi will either move to the
//respective area or do nothing.

void Application_Move(Application* app, HAL* hal)
{
    Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
    if (app->Eng > 0)
    {
        if (app->instruction == 'w')
        {
            if ((app->x == 20 && app->y == 75) || (app->x == 54 && app->y == 75) || (app->x == 88 && app->y == 75))
            {
                app->x = app->x;
                app->y = app->y - 45;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 10, app->y + 55, 17);
                app->num_Moves++;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 's')
        {
            if ((app->x == 20 && app->y == 30) || (app->x == 54 && app->y == 30) || (app->x == 88 && app->y == 30))
            {
                app->x = app->x;
                app->y = app->y + 45;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 10, app->y - 35, 17);
                app->num_Moves++;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 'a')
        {
            if ((app->x == 88 && app->y == 30) || (app->x == 88 && app->y == 75) || (app->x == 54 && app->y == 30) || (app->x == 54 && app->y == 75))
            {
                app->x = app->x - 34;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 44, app->y + 15, 17);
                app->num_Moves++;
                app->instruction = NULL;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 'd')
        {
            if ((app->x == 20 && app->y == 30) || (app->x == 54 && app->y == 30) || (app->x == 20 && app->y == 75) || (app->x == 54 && app->y == 75))
            {
                app->x = app->x + 34;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x - 24, app->y + 10, 17);
                app->num_Moves++;
                app->instruction = NULL;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Piplup4BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }

        }
    }


}
//Applicatiom_Move2 is the same as Application_Move but for when the Tamagotchi is in the child state
void Application_Move2(Application* app, HAL* hal)
{
    Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
    if (app->Eng > 0)
    {
        if (app->instruction == 'w')
        {
            if ((app->x == 20 && app->y == 75) || (app->x == 54 && app->y == 75) || (app->x == 88 && app->y == 75))
            {
                app->x = app->x;
                app->y = app->y - 45;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 10, app->y + 55, 17);
                app->num_Moves++;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 's')
        {
            if ((app->x == 20 && app->y == 30) || (app->x == 54 && app->y == 30) || (app->x == 88 && app->y == 30))
            {
                app->x = app->x;
                app->y = app->y + 45;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 10, app->y - 35, 17);
                app->num_Moves++;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 'a')
        {
            if ((app->x == 88 && app->y == 30) || (app->x == 88 && app->y == 75) || (app->x == 54 && app->y == 30) || (app->x == 54 && app->y == 75))
            {
                app->x = app->x - 34;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 44, app->y + 15, 17);
                app->num_Moves++;
                app->instruction = NULL;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 'd')
        {
            if ((app->x == 20 && app->y == 30) || (app->x == 54 && app->y == 30) || (app->x == 20 && app->y == 75) || (app->x == 54 && app->y == 75))
            {
                app->x = app->x + 34;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x - 24, app->y + 10, 17);
                app->num_Moves++;
                app->instruction = NULL;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Prinplup8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }

        }
    }


}
//Applicatiom_Move3 is the same as Application_Move but for when the Tamagotchi is in the adult state
void Application_Move3(Application* app, HAL* hal)
{
    Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
    if (app->Eng > 0)
    {
        if (app->instruction == 'w')
        {
            if ((app->x == 20 && app->y == 75) || (app->x == 54 && app->y == 75) || (app->x == 88 && app->y == 75))
            {
                app->x = app->x;
                app->y = app->y - 45;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 10, app->y + 55, 17);
                app->num_Moves++;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 's')
        {
            if ((app->x == 20 && app->y == 30) || (app->x == 54 && app->y == 30) || (app->x == 88 && app->y == 30))
            {
                app->x = app->x;
                app->y = app->y + 45;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 10, app->y - 35, 17);
                app->num_Moves++;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 'a')
        {
            if ((app->x == 88 && app->y == 30) || (app->x == 88 && app->y == 75) || (app->x == 54 && app->y == 30) || (app->x == 54 && app->y == 75))
            {
                app->x = app->x - 34;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x + 44, app->y + 15, 17);
                app->num_Moves++;
                app->instruction = NULL;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }
        }
        if (app->instruction == 'd')
        {
            if ((app->x == 20 && app->y == 30) || (app->x == 54 && app->y == 30) || (app->x == 20 && app->y == 75) || (app->x == 54 && app->y == 75))
            {
                app->x = app->x + 34;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                Graphics_setForegroundColor(&app->gfx.context, 16777215);
                Graphics_fillCircle(&app->gfx.context, app->x - 24, app->y + 10, 17);
                app->num_Moves++;
                app->instruction = NULL;
            }
            else
            {
                app->x = app->x;
                app->y = app->y;
                Graphics_drawImage(&app->gfx.context, &Empoleon8BPP_UNCOMP, app->x, app->y);
                app->num_Moves = app->num_Moves;
            }

        }
    }


}
