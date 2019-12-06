/*
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * TODO: loop through recv-functions (for clock-sync-data) until interrupted by mic-measurement -> send-function
 */

/*
 *    ======== tcpEcho.c ========
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

 /* NDK Header files */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#define TCPPACKETSIZE 1024
// #define TCPPORT 5000
#define NUMTCPWORKERS 3
#define SERVERIP "192.168.1.103"  // zuhause
// #define SERVERIP "169.254.219.29"  // technikum

/* Prototypes */



//*****************************************************************************
//
// System clock rate in Hz.
//
//*****************************************************************************
uint32_t g_ui32SysClock;


void delay(void)
{
	SysCtlDelay(g_ui32SysClock);
}

Void recvHandler(UArg arg0)
{
    fdOpenSession(TaskSelf());
    fdShare((SOCKET)arg0);
    char bufferRecv[40];
    int bytesRecv = 0;
    while(1)
    {
    bytesRecv = NDK_recv((SOCKET)arg0, bufferRecv, 40, MSG_DONTWAIT);
    if(bytesRecv == -1)
      {
        System_printf("recvHandler: socket failed with errorNo: %d\n", fdError());
        System_flush();
        Task_exit();
      }
    if(bytesRecv > 0)
        {
        System_printf("Received: %s\n", bufferRecv);
        System_flush();
        break;
        }
    bytesRecv = 0;
    }
    fdClose((SOCKET)arg0);
    // fdCloseSession(TaskSelf());
}

Void sendHandler(UArg arg0)
{
   // fdOpenSession(TaskSelf());
    fdShare((SOCKET)arg0);
    System_printf("socket: %d\n", (int)arg0);
       System_flush();
    char buffer[40] = "Endlich kommt der scheiss an!\n";
    // System_flush();
    int bytesSent = 1;
    size_t bufferSize_bytes = sizeof(buffer) / sizeof(buffer[0]);

    System_printf("bytesSent: %d\n", bytesSent);
    System_printf("bufferSize_bytes: %d\n", bufferSize_bytes);
    System_flush();

    bytesSent = NDK_send((SOCKET)arg0, (char *)buffer, bufferSize_bytes, 0 );
    if(bytesSent != bufferSize_bytes)
    {
        System_printf("sendHandler: send() --- bytesSent != bufferSize\n");
        System_flush();
                    Task_exit();
    }
    System_printf("bytesSent: %d\n", bytesSent);
    System_flush();

    System_printf("sent: %s\n", buffer);
    System_flush();
    fdClose((SOCKET)arg0);
    // fdCloseSession(TaskSelf());
}

/*
 *  ======== tcpHandler ========
 *  Creates new Task to handle new TCP connections.
 */
Void tcpHandler(UArg arg0, UArg arg1)
{
    SOCKET socket;
    struct sockaddr_in sLocalAddr;
    int errorNo;
    int connectLoopsCounter = 0;

    if(fdOpenSession(TaskSelf()) != 1){
        System_printf("error:fdOpenSession\n");
        System_flush();
    }

    socket = NDK_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) {
        errorNo = fdError();
        System_printf("tcpHandler: socket failed with errorNo: %d\n", errorNo);
        System_flush();
        Task_exit();
        return;
    }
    else {
        System_printf("Socket established: %d\n", (int)socket);
        System_flush();
    }

    memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
        const int server_port = 4444;
        // memset(&sLocalAddr, 0, sizeof(sLocalAddr));
        sLocalAddr.sin_family = AF_INET;
        sLocalAddr.sin_port = htons(server_port);
    if(inet_aton(SERVERIP, &sLocalAddr.sin_addr.s_addr) == 0) {
        System_printf("error: inet_aton\n");//htonl(INADDR_ANY);
        System_flush();
    }
    System_printf("Ip-Addr: %d\n", sLocalAddr.sin_addr.s_addr);
    System_flush();

    // try to connect; after 50 attempts -> exit
    delay();
    while(NDK_connect(socket, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr)) != 0){
    	if(connectLoopsCounter == 50) {
            errorNo = fdError();
            System_printf("tcpHandler: connect failed with errorNo: %d\n", errorNo);
            System_flush();
            Task_exit();
    	}
        //SysCtlDelay(g_ui32SysClock/100/3);
    	connectLoopsCounter++;
    	System_printf("loop-count: %d\n",connectLoopsCounter);
    	System_flush();
    }

    System_printf("connected to server\n");
    System_flush();

    // init task to send data

    Task_Handle taskHandleSend;
    Task_Params taskParamsSend;
    Error_Block ebSend;

    Task_Params_init(&taskParamsSend);
    Error_init(&ebSend);

    taskParamsSend.stackSize = 2048;
    taskParamsSend.priority = 2;
    taskParamsSend.arg0 = (UArg)socket;
    taskHandleSend = Task_create((Task_FuncPtr)sendHandler, &taskParamsSend, &ebSend);
    if (taskHandleSend == NULL) {
        System_printf("main: Failed to create sendHandler Task\n");
        System_flush();
    } else {
        System_printf("Sendhandler init.\n");
        System_flush();
    }

    // init task to receive data

    Task_Handle taskHandleRecv;
    Task_Params taskParamsRecv;
    Error_Block ebRecv;

    Task_Params_init(&taskParamsRecv);
    Error_init(&ebRecv);

    taskParamsRecv.stackSize = 2048;
    taskParamsRecv.priority = 1;
    taskParamsRecv.arg0 = (UArg)socket;
    taskHandleRecv = Task_create((Task_FuncPtr)recvHandler, &taskParamsRecv, &ebRecv);
    if (taskHandleRecv == NULL) {
        System_printf("main: Failed to create sendHandler Task\n");
        System_flush();
    } else {
        System_printf("Recvhandler init.\n");
        System_flush();
    }

   // fdCloseSession(TaskSelf());
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

	g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
	                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
	                SYSCTL_CFG_VCO_480), 120000000);

    /* Call board init functions */
    Board_initGeneral();
	Board_initGPIO();
    Board_initEMAC();

    System_printf("Starting the TCP\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /*
     *  Create the Task that farms out incoming TCP connections.
     *  arg0 will be the port that this task listens to.
     */
    Task_Params_init(&taskParams);
    Error_init(&eb);

    taskParams.stackSize = 4096;
    taskParams.priority = 2;
   // taskParams.arg0 = TCPPORT;
    taskHandle = Task_create((Task_FuncPtr)tcpHandler, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("main: Failed to create tcpHandler Task\n");
    } else {
        System_printf("TCPhandler init.\n");
    }
    System_flush();


    /* Start BIOS */
    BIOS_start();

    return (0);
}
