
/*! \file UART_Task.h
    \brief UART task
    \author Philip Albrecht

    ADC Task

*/

#ifndef ADC_TASK_H_
#define ADC_TASK_H_


/*! \fn adc_send_Fxn
 *  \brief Analog to digital converter & send over network
 *
 *
 *   \param socket Socket to send values to
 *   \param decibels decibels picked up from microphone
 *   \param decibels_calibrated decibel threshold
 *
 */
void adc_send_Fxn(UArg arg0);


/*! \fn setup_ADC_and_SEND_Task
 *  \brief Setup UART task
 *
 *  Setup UART task
 *  Task has highest priority and receives 1kB of stack
 *
 *  \return always zero. In case of error the system halts.
 */
int setup_ADC_and_SEND_Task(UArg arg0);

#endif /* ADC_TASK_H_ */
