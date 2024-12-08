/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PIControl.h"
#include "stdint.h"
#include "stdbool.h"
#include "../inc/MotorControl.h"
#include "../inc/Tachometer.h"
#include "../inc/Timer2A.h"
#include "../inc/Timer5A.h"

/* ================================================== */
/*            GLOBAL VARIABLE DEFINITIONS             */
/* ================================================== */
//FIFO STUFF
#define Periods_Timer0_FIFO_SIZE 100
PIControl_FIFO_DATATYPE Periods_Timer0_FIFO[Periods_Timer0_FIFO_SIZE];
PIControl_FIFO_DATATYPE Periods_Timer0_FIFO_head_ptr = 0; 
PIControl_FIFO_DATATYPE Periods_Timer0_FIFO_tail_ptr = 0;

// FIFO Buffer for Timer 3
#define Periods_Timer3_FIFO_SIZE 100
PIControl_FIFO_DATATYPE Periods_Timer3_FIFO[Periods_Timer3_FIFO_SIZE];
PIControl_FIFO_DATATYPE Periods_Timer3_FIFO_head_ptr = 0;
PIControl_FIFO_DATATYPE Periods_Timer3_FIFO_tail_ptr = 0;

#define PI_LOOP_PERIOD 800000
/* ================================================== */
/*            FUNCTION PROTOTYPES (DECLARATIONS)      */
/* ================================================== */
void clearFIFOs(void);

/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */

void PeriodicTask_PI_Loop(void){
		Tachometer_CheckForStopped();
		int32_t LeftU= MC_GetLeftU();
		int32_t RightU= MC_GetRightU();
        SetDutyCycles(RightU, LeftU);
        //GPIO_PORTE_DATA_R ^= 0x02;

}


void PI_Control_Init(void){
    clearFIFOs();
    Timer5A_Init(PeriodicTask_PI_Loop, PI_LOOP_PERIOD ,1);//execute PI every 10 ms with priority 1
}

void clearFIFOs(void){
    for(int i = 0; i < Periods_Timer0_FIFO_SIZE; i++){
        Periods_Timer0_FIFO[i] = 0;
        Periods_Timer3_FIFO[i] = 0;
    }
}

bool Put_Period_FIFO_Timer0(PIControl_FIFO_DATATYPE in){
    PIControl_FIFO_DATATYPE next_ptr = Periods_Timer0_FIFO_head_ptr + 1;
    next_ptr = next_ptr >= Periods_Timer0_FIFO_SIZE ? 0 : next_ptr;
    if(next_ptr == Periods_Timer0_FIFO_tail_ptr){
        return false; //is full
    }

    Periods_Timer0_FIFO[Periods_Timer0_FIFO_head_ptr] = in;
    Periods_Timer0_FIFO_head_ptr = next_ptr;
    return true;
}
//returns 0 if empty, 1 if not
bool Pop_Period_FIFO_Timer0(PIControl_FIFO_DATATYPE* Period_Out){
    if(Periods_Timer0_FIFO_tail_ptr == Periods_Timer0_FIFO_head_ptr){
        return false;//empty
    }
    *Period_Out = Periods_Timer0_FIFO[Periods_Timer0_FIFO_tail_ptr];
    Periods_Timer0_FIFO_tail_ptr++;
    //wrap around
    Periods_Timer0_FIFO_tail_ptr = Periods_Timer0_FIFO_tail_ptr >= Periods_Timer0_FIFO_SIZE ? 0 : Periods_Timer0_FIFO_tail_ptr;
    return true;
}

// Put function for Timer 3 FIFO
bool Put_Period_FIFO_Timer3(PIControl_FIFO_DATATYPE in) {
    PIControl_FIFO_DATATYPE next_ptr = Periods_Timer3_FIFO_head_ptr + 1;
    next_ptr = next_ptr >= Periods_Timer3_FIFO_SIZE ? 0 : next_ptr;

    // Check if the FIFO is full
    if (next_ptr == Periods_Timer3_FIFO_tail_ptr) {
        return false; // FIFO is full
    }

    // Insert the new sample and update head pointer
    Periods_Timer3_FIFO[Periods_Timer3_FIFO_head_ptr] = in;
    Periods_Timer3_FIFO_head_ptr = next_ptr;
    return true;
}

// Pop function for Timer 3 FIFO
bool Pop_Period_FIFO_Timer3(PIControl_FIFO_DATATYPE* Period_Out) {
    // Check if the FIFO is empty
    if (Periods_Timer3_FIFO_tail_ptr == Periods_Timer3_FIFO_head_ptr) {
        return false; // FIFO is empty
    }

    // Retrieve the sample and update the tail pointer
    *Period_Out = Periods_Timer3_FIFO[Periods_Timer3_FIFO_tail_ptr];
    Periods_Timer3_FIFO_tail_ptr++;

    // Wrap around if necessary
    Periods_Timer3_FIFO_tail_ptr = Periods_Timer3_FIFO_tail_ptr >= Periods_Timer3_FIFO_SIZE ? 0 : Periods_Timer3_FIFO_tail_ptr;
    return true;
}