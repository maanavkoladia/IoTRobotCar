#ifndef PICONTROL_H
#define PICONTROL_H

/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include "stdint.h" 
#include "stdbool.h"
/* ================================================== */
/*            GLOBAL VARIABLE DECLARATIONS            */
/* ================================================== */
#define PIControl_FIFO_DATATYPE uint32_t

/* ================================================== */
/*                 FUNCTION PROTOTYPES                */
/* ================================================== */
void PI_Control_Init(void);

// Function prototypes for Timer 0 FIFO
bool Put_Period_FIFO_Timer0(PIControl_FIFO_DATATYPE in);
bool Pop_Period_FIFO_Timer0(PIControl_FIFO_DATATYPE* Period_Out);

// Function prototypes for Timer 3 FIFO
bool Put_Period_FIFO_Timer3(PIControl_FIFO_DATATYPE in);
bool Pop_Period_FIFO_Timer3(PIControl_FIFO_DATATYPE* Period_Out);
#endif 

