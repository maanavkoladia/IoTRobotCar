// -------------------------------------------------
// CLI.h
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
// Functions:
// - CLI_Init: Initializes UART for CLI.
// - WaitForCMD: Waits for and processes a command from the user.
// - ProcessInputCMD: Parses the command and its parameters.
// - serveCMD: Executes the parsed command.
// - CLI_SystemStatus: Placeholder function for displaying system status.
// - clearProcessedBuf: Clears the processed command buffer.
//
// Usage Guide:
//
// This CLI accepts commands to control motor speed and tuning parameters for the
// Proportional-Integral (PI) controller. Commands are entered through the UART interface,
// where each command is followed by a parameter (if required) and a newline character.
//
// Commands:
// - `speed <value>`
//     - Sets the motor speed to the specified value.
//     - Example: `speed 100`
//
// - `pictl <parameter> <value>`
//     - Sets PI controller parameters, where <parameter> is one of:
//         - `kp1`: Proportional gain numerator
//         - `kp2`: Proportional gain denominator
//         - `ki1`: Integral gain numerator
//         - `ki2`: Integral gain denominator
//     - Example: `pictl kp1 50`
//
// Error Messages:
// - "Invalid speed parameter": Appears if `speed` is given a non-integer value.
// - "Invalid subcommand or parameter for pictl": Appears if `pictl` is used incorrectly,
//    with an unrecognized subcommand or non-integer value.
// - "Stop screwing with the CLI shitter": Appears for any unrecognized command.
//
// Example Session:
// User inputs:
//   - `speed 120`
//   - Response: "Speed set successfully."
//
//   - `pictl kp1 30`
//   - Response: "kp1 set successfully."
//
// Additional Notes:
// - `isValidInteger`: Helper function to check if a string is a valid integer.
// - `isDigit`: Checks if a character is a digit (0-9).
// - `strcmp`: Compares two strings.
// - `clearProcessedBuf`: Clears the buffer for storing processed command parameters.
//
// -------------------------------------------------
#ifndef CLI_H
#define CLI_H

/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdio.h>
#include <stdint.h>
/* ================================================== */
/*            GLOBAL VARIABLE DECLARATIONS            */
/* ================================================== */
#define MAX_CMD_INSTRING_LENGTH 30
/* ================================================== */
/*                 FUNCTION PROTOTYPES                */
/* ================================================== */
//-------------------   CLI_Init  -------------------
// Configure UART0 for serial full duplex operation
// Inputs: none
// Outputs: none
void CLI_Init(void);

void CLI_SystemStatus(void);
// -----------------    CLI_Process   ----------------------------
// This routine is called periodically. It is a command line interface
// to enter values to control the Motor 
// processed one character at a time, does not spin
// There are 6 states that this routine can be in:
//      cmd_state == 0  -> RESET
//      cmd_state == 1  -> Enter new_speed
//      cmd_state == 2  -> Enter Kp1
//      cmd_state == 3  -> Enter Kp2
//      cmd_state == 4  -> Enter Ki1
//      cmd_state == 5  -> Enter Ki1
void WaitForCMD(void);

#endif 




