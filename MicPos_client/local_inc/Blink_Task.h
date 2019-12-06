/*! \file Blink_Task.h
    \brief Blink task
    \author Matthias Wenzl
    \author Michael Kramer

    Blinking LED Task example.

*/

#ifndef BLINK_TASK_H_
#define BLINK_TASK_H_

#include <stdbool.h>
#include <stdint.h>
#include <xdc/std.h>


void ledFxn(UArg uarg0, UArg uarg1);


int setup_LED_task(UArg uarg0);

#endif
