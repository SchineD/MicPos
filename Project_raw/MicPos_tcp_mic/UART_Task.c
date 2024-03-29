/*
 *  ======== UART_Task.c ========
 *  Author: Michael Kramer / Matthias Wenzl
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>

/* TI-RTOS Header files */
#include <driverlib/sysctl.h>
#include <ti/drivers/UART.h>

/* Driverlib headers */
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>

/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/* Application headers */
#include "UART_Task.h"


/*
 *  ======== UART  ========
 *  Echo Characters recieved and show reception on Port N Led 0
 */


void UARTFxn(UArg arg0, UArg arg1)
{
    float output = 0;
    UART_Handle uart;
    UART_Params uartParams;
    Mailbox_Handle mbox;
    const char echoPrompt[] = "\fEchoing characters:\r\n";

    mbox = (Mailbox_Handle)arg0;


    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 9600;
    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        System_abort("Error opening the UART");
    }

    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    while (1) {


          // UART_read(uart, &input, 1);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 1);

           Mailbox_pend(mbox, &output, BIOS_NO_WAIT);

           char buffer [sizeof(output)+1] ;
           sprintf (buffer, "%f\r\n", output) ;  // print into buffer

           UART_write(uart, buffer, sizeof(buffer)); // Remove this line to stop echoing!
           Task_sleep(500);
           GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
       }
}


/*
 *  Setup task function
 */
int setup_UART_Task(UArg arg0)
{
    Task_Params taskUARTParams;
    Task_Handle taskUART;
    Error_Block eb;

    /* Enable and configure the peripherals used by the UART0 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UART_init();

    /* Setup PortN LED1 activity signaling */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    Error_init(&eb);
    Task_Params_init(&taskUARTParams);
    taskUARTParams.stackSize = 2048; /* stack in bytes */
    taskUARTParams.priority = 15; /* 0-15 (15 is highest priority on default -> see RTOS Task configuration) */
    taskUARTParams.arg0 = arg0;//=mbox

    taskUART = Task_create((Task_FuncPtr)UARTFxn, &taskUARTParams, &eb);
    if (taskUART == NULL) {
        System_abort("TaskUART create failed");
    }

    return (0);
}
