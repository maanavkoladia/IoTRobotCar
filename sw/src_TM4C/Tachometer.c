// Tachmeter.c
// Runs on TM4C123 (not LF120)
// Jonathan Valvano
// July 1, 2024

// tachometer
//   TM4C MSPM0
//   PB7  PB8  ELA  T0CCP1
//   PB2  PB12 ERA  T3CCP0
//             ERB  GPIO input  (not used)
//             ELB  GPIO input  (not used)


/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/Tachometer.h"
#include "../inc/InputCapture.h"
#include "../inc/PIControl.h"
#include "../inc/Lab8.h"
#include "../inc/MotorControl.h"
#include "../inc/Timer1A.h"
    

/* ================================================== */
/*            GLOBAL VARIABLE DEFINITIONS             */
/* ================================================== */
#define RPM_DIV_CONSTANT 133333333 //tried 400 mil, no. trying 66 mil repeating because 720
uint32_t Period_T0;
uint32_t Previous_T0;

uint32_t Period_T3;
uint32_t Previous_T3;

int CountTimer0=0;
int SpeedFlagR=0;
int CountTimer3=0;
int SpeedFlagL=0;

#define dumpPERIOD 800000
uint32_t rpms_dump_buf[RMPS_DUMP_BUF_SIZE];
uint32_t rpms_dump_buf_idx = 0;
/* ================================================== */
/*            FUNCTION PROTOTYPES (DECLARATIONS)      */
/* ================================================== */


/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */

void IntTask_ReadPeriod_Timer0B(void) {
    uint32_t current_T0 = TIMER0_TBR_R; // Read TIMER0_TBR_R once to ensure consistency
    CountTimer0=0;//used for checked for stopped
    // Calculate the period, accounting for potential wraparound
    if (current_T0 <= Previous_T0) {
        // Timer has not wrapped around
        Period_T0 = (Previous_T0 - current_T0) & 0xFFFFFFFF;
    } else {
        // Timer has wrapped around: adjust the calculation
        Period_T0 = ((0xFFFFFFFF - current_T0) + Previous_T0 + 1) & 0xFFFFFFFF;
    }
    
    Previous_T0 = current_T0; // Update for the next calculation
    
    // Attempt to put Period_T0 into FIFO and handle overflow if FIFO is full
    if (!Put_Period_FIFO_Timer0(Period_T0)) {
        //GPIO_PORTE_DATA_R |= 0x02;   // Set flag on overflow
        //fifofull_flag_cnt++;         // Increment overflow count
    }
}


void IntTask_ReadPeriod_Timer3A(void) {
    uint32_t current_T3 = TIMER3_TAR_R; // Read TIMER3_TAR_R once to ensure consistency
    CountTimer3 = 0; // Reset count to indicate that the motor is not stopped

    // Calculate the period, accounting for potential wraparound
    if (current_T3 <= Previous_T3) {
        // Timer has not wrapped around
        Period_T3 = (Previous_T3 - current_T3) & 0xFFFFFFFF;
    } else {
        // Timer has wrapped around: adjust the calculation
        Period_T3 = ((0xFFFFFFFF - current_T3) + Previous_T3 + 1) & 0xFFFFFFFF;
    }

    Previous_T3 = current_T3; // Update Previous_T3 for the next calculation

    // Attempt to put Period_T3 into FIFO and handle overflow if FIFO is full
    if (!Put_Period_FIFO_Timer3(Period_T3)) {
        //GPIO_PORTE_DATA_R |= 0x02;   // Set flag on overflow
    }
}

void ClearCounts(void){
		CountTimer0 = 0;CountTimer3 = 0;
}
// Function to check if the motors are stopped
void Tachometer_CheckForStopped(void) {
    // Add logic to check if motors have stopped
    // This could involve checking for the absence of input capture interrupts
	CountTimer0++;	CountTimer3++;
	
	if(CountTimer0>=12) {SpeedFlagR=1;MC_SetDesiredSpeed(1);CountTimer0=0;}
	if(CountTimer3>=12) {SpeedFlagL=1;MC_SetDesiredSpeed(1);CountTimer3=0;}
}

// Function to initialize the tachometers for RSLK motors
void Tachometer_Init(void) {
    TimerCapture0B_Init(IntTask_ReadPeriod_Timer0B);
    TimerCapture3_Init(IntTask_ReadPeriod_Timer3A);
    //Timer1A_Init(PeriodicTask_dumpRPMS, dumpPERIOD, 0);
    // Initialization code for the tachometers
    // Setup input capture on ELA and ERA pins for both motors
}


// Function to get the speed of the right motor in 0.1 rpm
uint32_t Tachometer_GetRightrpm(void) {
  		// Speed (rpm) = 400,000,000/Period
	uint32_t periods_sum=0;
	int32_t count=0;
	while(1){
		uint32_t Period_Out=0;
		if(Pop_Period_FIFO_Timer3(&Period_Out)==false){
			break;
		}
		else periods_sum+=Period_Out;
		count++;
	}
	uint32_t Period = periods_sum/count;
	if(Period>0){ 
		SpeedFlagR=0;
		Period = RPM_DIV_CONSTANT/Period;

        // rpms_dump_buf[rpms_dump_buf_idx] = Period;
        // if(rpms_dump_buf_idx < RMPS_DUMP_BUF_SIZE){
        //     rpms_dump_buf_idx++;
        // }

		return Period;
	}
	if(SpeedFlagR==1) return 0;//0 rpm
    return 0;
}

// Function to get the speed of the left motor in 0.1 rpm
uint32_t Tachometer_GetLeftrpm(void) {
   		// Speed (rpm) = 400,000,000/Period
	uint32_t periods_sum=0;
	int32_t count=0;
	while(1){
		uint32_t Period_Out=0;
		if(Pop_Period_FIFO_Timer0(&Period_Out)==false){
			break;
		}
		else periods_sum+=Period_Out;
		count++;
	}
	uint32_t Period = periods_sum/count;
	if(Period>0){
		SpeedFlagL=0;
        Period = RPM_DIV_CONSTANT/Period;

        

		return Period;
	}
	if(SpeedFlagL==1) return 0;//0 rpm
    return 0;
}
