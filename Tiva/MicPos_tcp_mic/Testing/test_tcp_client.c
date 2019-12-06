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
#include <math.h>

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
void test_Init();
int test_setup_ADC_and_Send_Task(UArg socket);




//*****************************************************************************
//
// System clock rate in Hz.
//
//*****************************************************************************
uint32_t g_ui32SysClock;


void test_delay(void)
{
	SysCtlDelay(g_ui32SysClock);
}

/*
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
*/

/*
 *  ======== tcpHandler ========
 *  Creates new Task to handle new TCP connections.
 */

Void test_tcpHandler(UArg arg0, UArg arg1)
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
    test_delay();
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

    test_setup_ADC_and_SEND_Task((UArg)socket);


   // fdCloseSession(TaskSelf());
}


void test_Init()
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
int test_main(void)
{

    //change this value to the one found in the Testplan times 10 (eg. for testscenario 2.3 set int testscenario = 23
    int testscenario;

    switch(testscenario){
        // Test 2.1 Tests for sample array to decibel calculation
        case 21:
        {
            int i = 0;
            int j = 0;
            int k = 0;
            float decibels = 0;
            uint32_t peakToPeak = 0;


           // uint32_t sample_array[3][10];

            uint32_t sample_array[3][10] = {
                {1940 ,1994 ,1953 ,1940 ,1964 ,2006 ,2047 ,2020 ,1940 ,1976},
                {1940 ,1994 ,1953 ,1940 ,1964 ,2006 ,2047 ,2020 ,1940 ,4976},
                {1940 ,1994 ,1953 ,1940 ,1964 ,2006 ,2047 ,2020 ,1940 ,3000},
            };
            float expected_output[3] = { 69.57813 , 136.4875, 115.4420 };

            // Test different starting peaks
            uint32_t test_low_peak[3] = { 1940 , 4000, 2000 };
            uint32_t test_high_peak[3] = { 1940 , 100, 3000 };

            for(k = 0; k < 3; k++){
                for(i = 0; i < 3; i++){
                    // needs to be reseted for every testcase
                    uint32_t low_peak = test_low_peak[k];
                    uint32_t high_peak = test_high_peak[k];
                    for(j = 0; j < 10; j++){
                         if(low_peak > sample_array[i][j]) low_peak = sample_array[i][j];
                         if(high_peak < sample_array[i][j]) high_peak = sample_array[i][j];
                         System_printf("low: %i\n    high: %i\n", low_peak,high_peak);
                         System_flush();
                    }

                    peakToPeak = high_peak - low_peak;
                    System_printf("peak2peak: %i\n", peakToPeak);
                    System_flush();
                    decibels = 20*log(((float)peakToPeak)/3.3);
                    System_printf("Decibels: %f\n    Decibels-expected: %f\n", decibels,expected_output[i]);
                    System_flush();
                }
            }
            break;
        }
        // Default behavior as seen in tcp_client.c
        default:
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
                taskHandle = Task_create((Task_FuncPtr)test_tcpHandler, &taskParams, &eb);
                if (taskHandle == NULL) {
                    System_printf("main: Failed to create tcpHandler Task\n");
                } else {
                    System_printf("TCPhandler init.\n");
                }
                System_flush();


                // ADC-Stuff
                test_Init();

                /* Start BIOS */
                BIOS_start();

                return (0);
            break;
        }
    }


}
