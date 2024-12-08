// -------------------------------------------------
// CLI.c
// 
// This UART port is for the Command Line Interface to
// Motor controller.
//
// Author:      Daniel Valvano
// Date:        May 23, 2014
// Modified by: Mark McDermott, Jonathan Valvano
// Date:        July 1, 2024
//
//-------------------------------------------------
//
// U0Rx connected to PA0
// U0Tx connected to PA1
/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <string.h>
//#include <ctype.h>  // for isdigit
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CLI.h"
#include "../inc/UART.h"
#include "../inc/MotorControl.h"
#include "../inc/UART0int.h"
#include "../inc/Tachometer.h"

/* ================================================== */
/*            GLOBAL VARIABLE DEFINITIONS             */
/* ================================================== */
#define NUMBER_OF_PARAMS 3
char UartIn_CMD_unprocessed_buf[MAX_CMD_INSTRING_LENGTH];

char UartIn_CMD_processed[NUMBER_OF_PARAMS][MAX_CMD_INSTRING_LENGTH];



/* ================================================== */
/*            FUNCTION PROTOTYPES (DECLARATIONS)      */
/* ================================================== */
bool isValidInteger(const char* buf, uint32_t* out);
uint8_t isDigit(char c);
int strcmp(const char *str1, const char *str2);
void clearProcessedBuf(void);
/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */
void CLI_Init(void) {
    // Empty function for CLI initialization
    UART_Init();
}


void ProcessInputCMD(void) {
    uint8_t param_idx = 0;  // Index for parameters
    uint16_t j = 0;         // Index for characters within each parameter

    for (uint16_t i = 0; i < MAX_CMD_INSTRING_LENGTH && UartIn_CMD_unprocessed_buf[i] != '\n' && UartIn_CMD_unprocessed_buf[i] != '\0'; i++) {
        if (UartIn_CMD_unprocessed_buf[i] == ' ') {
            // Null-terminate the current parameter
            UartIn_CMD_processed[param_idx][j] = '\0';
            
            // Move to the next parameter
            param_idx++;
            j = 0;  // Reset character index for the new parameter

            // Stop if we've reached the maximum parameter count
            if (param_idx >= NUMBER_OF_PARAMS) {
                break;
            }
        } else {
            // Add character to the current parameter, checking bounds
            if (j < MAX_CMD_INSTRING_LENGTH - 1) {  // Reserve space for null terminator
                UartIn_CMD_processed[param_idx][j] = UartIn_CMD_unprocessed_buf[i];
                j++;
            }
        }
    }

    // Null-terminate the last parameter if any characters were added
    UartIn_CMD_processed[param_idx][j] = '\0';
}


void serveCMD(void) {
    uint32_t numberParam;
    
    // Check if the command is "speed"
    if (strcmp(UartIn_CMD_processed[0], "speed") == 0) {
        // Validate and set speed
        if (isValidInteger(UartIn_CMD_processed[1], &numberParam)) {
            MC_SetDesiredSpeed(numberParam);
            UART_OutString("Speed set successfully.\n");
        } else {
            UART_OutString("\nStop screwing with the CLI shitter\n");
        }

    // Check if the command is "pictl"
    } else if (strcmp(UartIn_CMD_processed[0], "pictl") == 0) {
        // Process "pictl" subcommands
        if (strcmp(UartIn_CMD_processed[1], "kp1") == 0 && isValidInteger(UartIn_CMD_processed[2], &numberParam)) {
            MC_SetKp1(numberParam);
            UART_OutString("\n\tkp1 set successfully.\n");
        } else if (strcmp(UartIn_CMD_processed[1], "kp2") == 0 && isValidInteger(UartIn_CMD_processed[2], &numberParam)) {  
            MC_SetKp2(numberParam);
            UART_OutString("\n\tkp2 set successfully.\n");
        } else if (strcmp(UartIn_CMD_processed[1], "ki1") == 0 && isValidInteger(UartIn_CMD_processed[2], &numberParam)) {           
            MC_SetKi1(numberParam);
            UART_OutString("\n\tki1 set successfully.\n");
        } else if (strcmp(UartIn_CMD_processed[1], "ki2") == 0 && isValidInteger(UartIn_CMD_processed[2], &numberParam)) {         
            MC_SetKi2(numberParam);
            UART_OutString("\n\tki2 set successfully.\n");
        } else {
            UART_OutString("\nStop screwing with the CLI shitter\n");
        }

    } else if (strcmp(UartIn_CMD_processed[0], "p") == 0) {
			// print all k values
			UART_OutString("\nPrinting all values");
			UART_OutString("\n\tKi1: ");
			UART_OutUDec(MC_GetKi1());
			UART_OutString("\n\tKi2: ");
			UART_OutUDec(MC_GetKi2());
			UART_OutString("\n\tKp1: ");
			UART_OutUDec(MC_GetKp1());
			UART_OutString("\n\tKp2: ");
			UART_OutUDec(MC_GetKp2());
			UART_OutString("\n\tDesired Speed: ");
			UART_OutUDec(MC_GetDesiredSpeed());
			UART_OutString("\n\tTachometer Left Speed: ");
			UART_OutUDec(Tachometer_GetLeftrpm());
			UART_OutString("\n\tTachometer Right Speed: ");
			UART_OutUDec(Tachometer_GetRightrpm());
			UART_OutString("\n");
		} else if (
				strcmp(UartIn_CMD_processed[0], "monitor") == 0 ||
				strcmp(UartIn_CMD_processed[0], "m") == 0
		) {
			// Continuous monitoring of speed values
			UART_OutString("\n(J: Get values. Q: Quit");
			UART_OutString("\nDesired speed:");
			UART_OutUDec(MC_GetDesiredSpeed());
			int shouldLoop = true;
			while (shouldLoop) {
				switch(UART_InChar()) {
					case 'j': {
						UART_OutString("\nLeft: ");
						UART_OutUDec(Tachometer_GetLeftrpm());
						UART_OutString(" | Right: ");
						UART_OutUDec(Tachometer_GetRightrpm());
						break;
					}
					case 'q': {
						UART_OutString("\nDone Monitoring\n");
						shouldLoop = false;
						break;
					}
				}
			}
		} else if (strcmp(UartIn_CMD_processed[0], "calibrate") == 0) {
			// Calibration
			uint32_t goal_speed = 400;
			uint32_t base = 5000;
			uint32_t res = 1000; // how much to vary each k value (on first iteration)
			uint32_t range = 5; // how many k values to check
			
			int ki1, ki2, kp1, kp2;
			ki1 = base; // Best values for ki1 and kp1
			ki2 = 50000;
			kp1 = base;
			kp2 = 50000;
			unsigned ki1_base, kp1_base;
			MC_SetKi1(ki1);
			MC_SetKp1(kp1);
			MC_SetKi2(ki2);
			MC_SetKp2(kp2);
			MC_SetDesiredSpeed(goal_speed);
			
			int i = 0, j = 0, k = 0;
			unsigned sample_count = 10000;
			int best_error = 0x7fffffff; // INT_MAX
			while (res > 1) {
				unsigned best_ki1, best_kp1;
				int test_ki1 = ki1 - res * range;
				int ki1_range_end = ki1 + res * range;
				if (test_ki1 < 0) test_ki1 = 0;
				int test_kp1 = kp1 - res * range;
				int kp1_range_end = kp1 + res * range;
				if (test_kp1 < 0) test_kp1 = 0;
				int base_kp1 = test_kp1;
				UART_OutString("\n\tKi1 - testing vals between: ");
				UART_OutUDec(test_ki1);
				UART_OutString(" and ");
				UART_OutUDec(ki1_range_end);
				UART_OutString("\n\tKp1 - testing vals between: ");
				UART_OutUDec(test_kp1);
				UART_OutString(" and ");
				UART_OutUDec(kp1_range_end);
				while (test_ki1 <= ki1_range_end) {
					while (test_kp1 <= kp1_range_end) {
						// set k values and measure error
						unsigned error_sum = 0;
						MC_SetKi1(test_ki1);
						MC_SetKp1(test_kp1);
						UART_OutString("\n\tKi1: ");
						UART_OutUDec(MC_GetKi1());
						UART_OutString(" | Ki2: ");
						UART_OutUDec(MC_GetKi2());
						UART_OutString(" | Kp1: ");
						UART_OutUDec(MC_GetKp1());
						UART_OutString(" | Kp2: ");
						UART_OutUDec(MC_GetKp2());
						for (k = 0; k < sample_count; k++) {
							int left_error = MC_GetLeftE();
							int right_error = MC_GetRightE();
							error_sum += left_error >= 0 ? left_error : -left_error;
							error_sum += right_error >= 0 ? right_error : -right_error;
						}
						UART_OutString(" | Error: ");
						UART_OutUDec(error_sum);
						if (error_sum < best_error) {
							UART_OutString(" | a new best! ");
							ki1 = test_ki1;
							kp1 = test_kp1;
							best_error = error_sum;
						}
						test_kp1 += res;
					}
					UART_OutString("\n\tIncrement Ki");
					test_ki1 += res;
					test_kp1 = base_kp1;
				}
				UART_OutString("\n\tReducing resolution");
				res /= 2;
			}
			
			
		}
		else if(strcmp(UartIn_CMD_processed[0], "checkstopped") == 0){
			UART_OutString("\nLeft Motor Stopped?");
			UART_OutUDec(SpeedFlagL);
			UART_OutString("\nRight Motor Stopped?");
			UART_OutUDec(SpeedFlagR);		
		}
		else {
        // Unrecognized command feedback
        UART_OutString("\nStop screwing with the CLI shitter\n");
    }
    clearProcessedBuf();
}

void clearProcessedBuf(void) {
    for (uint8_t i = 0; i < NUMBER_OF_PARAMS; i++) {
        UartIn_CMD_processed[i][0] = '\0';  // Null-terminate each string
    }
}


bool isValidInteger(const char* buf, uint32_t* out) {
    const char* start = buf; // Keep a pointer to the start for conversion
    if(*buf == '\0'){return false;}
    // Check if each character is a digit
    while (*buf) {
        if (!isDigit((unsigned char)*buf)) {
            return false;  // If any character is not a digit, return false
        }
        buf++;
    } 
    
    // Convert the validated string to an integer
    *out = atoi(start);  // Convert from the start of the string
    return true;  // Return true to indicate successful validation and conversion
}

uint8_t isDigit(char c) {
    return (c >= '0' && c <= '9');
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}


void CLI_SystemStatus(void) {
    // Empty function to display system status
}

void WaitForCMD(void){
    UART_InString(UartIn_CMD_unprocessed_buf, MAX_CMD_INSTRING_LENGTH);
    //GPIO_PORTF_DATA_R |= 0x08;
    ProcessInputCMD();
    serveCMD();
}



   


