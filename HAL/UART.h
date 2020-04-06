/*
 * UART.h
 *
 *  Created on: Dec 31, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef HAL_UART_H_
#define HAL_UART_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// Ever since the new API update in mid 2019, the UART configuration struct's
// name changed from [eUSCI_UART_Config] to [eUSCI_UART_ConfigV1]. If your code
// does not compile, uncomment the alternate #define and comment the original
// one.
// -----------------------------------------------------------------------------
// #define UART_Config eUSCI_UART_Config
#define UART_Config eUSCI_UART_ConfigV1

// An enum outlining what baud rates the UART_construct() function can use in
// its initialization.
enum _UART_Baudrate
{
    BAUD_9600, BAUD_19200, BAUD_38400, BAUD_57600, NUM_BAUD_CHOICES
};
typedef enum _UART_Baudrate UART_Baudrate;

// TODO: Write an overview explanation of what this UART struct does, and how it
//       interacts with the functions below. Consult <HAL/Button.h>,
//       <HAL/LED.h>, and <HAL/Timer.h> for examples on how to do this.
struct _UART
{
    UART_Config config;

    uint32_t moduleInstance;
    uint32_t port;
    uint32_t pins;
};
typedef struct _UART UART;
//This struct allows us to select what ports and pins UART will go out through.
//Also, you can set the module instance which is always EUSCI_A0_BASE.

// Initializes a UART connection.
UART UART_construct(UART_Baudrate baudChoice);

// TODO: Write a comment which explains what each of these functions does. In the
//       header, prefer explaining WHAT the function does, as opposed to HOW it is
//       implemented.

char UART_getChar(UART* uart);
//UART_getChar take in the character

bool UART_hasChar(UART* uart);
// UART_hasChar makes sure the getChar function was sucessful in receiving a character

bool UART_canSend(UART* uart);
//Uart_canSend send the character to the MobaXterm program

void UART_putChar(UART* uart, char c);
//UART_putChar put the correct character in the screen in the MobaXterm program

// Updates the UART baudrate to use the new baud choice.
void UART_updateBaud(UART* uart, UART_Baudrate baudChoice);

#endif /* HAL_UART_H_ */
