
//*****************************************************************************
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Task.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

#include "driverlib/sysctl.h"
#include "time.h"
#include "driverlib/timer.h"

#define TIVABOARD 'A'

int setup_ADC_and_SEND_Task(UArg socket);


void adc_send_Fxn(UArg arg0)
{
    // setup timer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
                   while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
                   {
                   }
                   TimerConfigure(TIMER0_BASE, (TIMER_CFG_PERIODIC));
                   TimerPrescaleSet(TIMER0_BASE, TIMER_A, 1000);
                   TimerEnable(TIMER0_BASE, TIMER_A);

                   //Set Timer Value
                      TimerLoadSet(TIMER0_BASE, TIMER_A, 80000);
                      System_printf("Timer is set\n");
                              System_flush();

    SOCKET socket = (SOCKET)arg0;
    uint32_t ui32Value_calibrated = 0;
    int t = 0;
    int j = 0;
    int k = 0;
    float decibels = 0;
    float decibels_calibrated = 0;
    uint32_t low_peak = 0;
    uint32_t high_peak = 0;
    uint32_t peakToPeak = 0;
    uint32_t sample_array[10];
    uint32_t timerValue;

    ADCProcessorTrigger(ADC0_BASE, 3);
    while(!ADCIntStatus(ADC0_BASE, 3, false)){};
    ADCSequenceDataGet(ADC0_BASE, 3, &ui32Value_calibrated);

    for(t = 0; t < 10; t++){
    sample_array[t] = ui32Value_calibrated;
    }
    low_peak = ui32Value_calibrated;
    high_peak = ui32Value_calibrated;

    // fill array with samples
    for(j = 0; j < 10; j++){
        ADCProcessorTrigger(ADC0_BASE, 3);
        while(!ADCIntStatus(ADC0_BASE, 3, false)){};
                ADCSequenceDataGet(ADC0_BASE, 3, &sample_array[j]);
                if(low_peak > sample_array[j]) low_peak = sample_array[j];
                if(high_peak < sample_array[j]) high_peak = sample_array[j];
            }

     peakToPeak = high_peak - low_peak;
     decibels_calibrated = 20*log(((float)peakToPeak)/3.3);

    while(true){

         //continously calculate peak-to-peak
         for(k = 0; k < 10; k++)
         {
             ADCProcessorTrigger(ADC0_BASE, 3);
             while(!ADCIntStatus(ADC0_BASE, 3, false)){};
             ADCSequenceDataGet(ADC0_BASE, 3, &sample_array[k]);
             if(low_peak > sample_array[k]) low_peak = sample_array[k];
             if(high_peak < sample_array[k]) high_peak = sample_array[k];
         }
         peakToPeak = high_peak - low_peak;
         decibels = 20*log(((float)peakToPeak)/3.3);

         System_printf("Decibels: %f\n", decibels);
         System_flush();

         if(decibels >= decibels_calibrated/100*140)
         {
             timerValue =  TimerValueGet(TIMER0_BASE, TIMER_A);
             // send Board-nr. and Timestamp to server
             fdShare(socket);
                 System_printf("socket: %d\n", (int)arg0);
                    System_flush();
                 char buffer[9];
                 snprintf(buffer,9,"%c %05d\n", TIVABOARD, timerValue);


                 int bytesSent = 1;
                 size_t bufferSize_bytes = sizeof(buffer) / sizeof(buffer[0]);

                 System_printf("bytesSent: %d\n", bytesSent);
                 System_printf("bufferSize_bytes: %d\n", bufferSize_bytes);

                 System_printf("TimerValue: %d\n", timerValue);
                 System_flush();

                 bytesSent = NDK_send(socket, (char *)buffer, bufferSize_bytes, 0 );
                 if(bytesSent != bufferSize_bytes)
                 {
                     System_printf("sendHandler: send() --- bytesSent != bufferSize\n");
                     System_flush();
                     fdClose(socket);
                                 Task_exit();
                 }
                 System_printf("bytesSent: %d\n", bytesSent);
                 System_flush();

                 System_printf("sent: %s\n", buffer);
                 System_flush();
                 Task_sleep(10000);
                 fdClose(socket);
         }

         low_peak = ui32Value_calibrated;
         high_peak = ui32Value_calibrated;
     }
}



int setup_ADC_and_SEND_Task(UArg arg0)
{



    Task_Params taskParams;
    Task_Handle taskAdc;
    Error_Block eb;

    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.stackSize = 8192;    /*stack in bytes*/
    taskParams.priority = 1;       /*0-15 (16 is highest priority) -> see RTOS configuration*/
    taskParams.arg0 =  (UArg) arg0; // = socket

    taskAdc = Task_create((Task_FuncPtr)adc_send_Fxn, &taskParams, &eb);
    if (taskAdc == NULL) {
        System_abort("Couldn't create task");
    }

    return 0;
}
