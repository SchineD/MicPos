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
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>

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
#include <UART_Task.h>

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
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"


#define TCPPACKETSIZE 1024
// #define TCPPORT 5000
#define NUMTCPWORKERS 3
// #define SERVERIP "192.168.1.103"  // zuhause
#define SERVERIP "169.254.47.214"
/* Prototypes */
void Init();
int setup_ADC_and_Send_Task(UArg socket);


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
    /*****for Multicast*******
    SOCKET socketRecv;
        struct sockaddr_in sLocalAddr;
        struct ip_mreq mreq;
        int errorNo;

        socketRecv = NDK_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket == INVALID_SOCKET) {
            errorNo = fdError();
            System_printf("tcpHandler: socketRecv failed with errorNo: %d\n", errorNo);
            System_flush();
            Task_exit();
            return;
        }
        else {
            System_printf("socketRecv established: %d\n", (int)socketRecv);
            System_flush();
        }

        int yes = 1;
           if (NDK_setsockopt(socketRecv, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0){
              System_printf("Reusing ADDR failed\n");
           }

        memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
        const int server_port = 5000;
        sLocalAddr.sin_family = AF_INET;
        sLocalAddr.sin_port = htons(server_port);
        if(inet_aton("192.168.1.105", &sLocalAddr.sin_addr.s_addr) == 0) {
             System_printf("error: inet_aton\n");//htonl(INADDR_ANY);
             System_flush();
           }
        System_printf("Ip-Addr: %d\n", sLocalAddr.sin_addr.s_addr);
        System_flush();

        int binderror = NDK_bind(socketRecv, (struct sockaddr *)&sLocalAddr, sizeof(sLocalAddr));
        if(binderror != 0){
            binderror = fdError();
            System_printf("%d\n", binderror);
        }
        // setup to recv multicast datagrams
        memset(&mreq,0,sizeof(mreq));
        mreq.imr_multiaddr.s_addr = inet_addr("238.255.255.240"); // group addr
        mreq.imr_interface.s_addr = inet_addr("192.168.1.105"); // use default

        //Make this a member of the Multicast Group
        if(NDK_setsockopt(socketRecv, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
        {
        System_printf("Error: multicast - setsockopt - add membership\n");
        }
        // set TTL (Time To Live)
        unsigned int ttl = 80;
        if (NDK_setsockopt(socketRecv, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(ttl) ) < 0)
        {
            System_printf("Error: multicast - setsockopt - ttl\n");
        }

    char bufferRecv[5];
    int bytesRecv = 0;
    struct sockaddr pName;
    int plen = sizeof(pName);

    // wait for message from server to start timer and mic in adc-send-task
    bytesRecv = NDK_recvfrom(socketRecv, bufferRecv, sizeof(bufferRecv), MSG_WAITALL, (struct sockaddr *)&pName, &plen);
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
        }
    bytesRecv = 0;
    fdClose((SOCKET)socketRecv);
    setup_ADC_and_SEND_Task((UArg)arg0);
    // fdCloseSession(TaskSelf());
     */

    fdOpenSession(TaskSelf());
        fdShare((SOCKET)arg0);
        char bufferRecv[5];
        int bytesRecv = 0;

        bytesRecv = NDK_recv((SOCKET)arg0, bufferRecv, 5, MSG_WAITALL);
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
            }
        bytesRecv = 0;

        fdClose((SOCKET)arg0);
        setup_ADC_and_SEND_Task((UArg)arg0);
        // fdCloseSession(TaskSelf());
}
}
*/

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

    Error_Block eb2;
    Error_init(&eb2);

    setup_ADC_and_SEND_Task((UArg)socket);


   // fdCloseSession(TaskSelf());
}


void Init()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }
    TimerConfigure(TIMER0_BASE, (TIMER_CFG_PERIODIC));
    TimerEnable(TIMER0_BASE, TIMER_A);

    TimerLoadSet(TIMER0_BASE, TIMER_A, 8000);

    TimerValueGet(TIMER0_BASE, TIMER_A);

    SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
    SysCtlDelay(10);
      while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
      {
      }

      ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
      ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH3);
      ADCIntEnable(ADC0_BASE, 3);
      ADCSequenceEnable(ADC0_BASE, 3);
      ADCSoftwareOversampleConfigure(ADC0_BASE, 3, (uint32_t) 1); //test

    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
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

    System_printf("Starting TCP\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();


    Task_Params_init(&taskParams);
    Error_init(&eb);

    taskParams.stackSize = 4096;
    taskParams.priority = 1;
    taskHandle = Task_create((Task_FuncPtr)tcpHandler, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("main: Failed to create tcpHandler Task\n");
    } else {
        System_printf("TCPhandler init.\n");
    }
    System_flush();


    // ADC-Stuff
    Init();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
