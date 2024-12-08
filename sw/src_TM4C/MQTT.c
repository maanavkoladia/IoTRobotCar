// ----------------------------------------------------------------------------
//
// File name: MQTT.c
//
// Description: This code is used to bridge the TM4C123 board and the MQTT Web Application
//              via the ESP8266 WiFi board

// Authors:       Mark McDermott
// Orig gen date: June 3, 2023
// Last update:   July 21, 2023
//
// ----------------------------------------------------------------------------
/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdint.h>
//#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/MotorControl.h"
#include "../inc/MQTT.h"
#include "../inc/Lab9.h"
#include "../inc/UART.h"
#include "../inc/UART2.h"
#include "../inc/UART5.h"
#include "../inc/esp8266.h"
#include "../inc/Timer1A.h"
#include "../inc/Timer2A.h"

/* ================================================== */
/*            GLOBAL VARIABLE DEFINITIONS             */
/* ================================================== */
//#define DEBUG1                // First level of Debug
#undef  DEBUG1                // Comment out to enable Debug1

#define UART5_FR_TXFF 0x20  // UART Transmit FIFO Full
#define UART5_FR_RXFE 0x10  // UART Receive FIFO Empty
#define UART5_LCRH_WLEN_8 0x00000060  // 8 bit word length
#define UART5_LCRH_FEN 0x00000010  // UART Enable FIFOs
#define UART5_CTL_UARTEN 0x00000001  // UART Enable

//extern uint32_t         Kp1; //Extern for any variables we need to set/get for MQTT code
//extern uint32_t         Kp2;
//extern uint32_t         Ki1;
//extern uint32_t         Ki2;
//extern uint32_t         Xstar;
#define RX_TERM_CHAR '\r'
uint32_t example_left_motor_speed;

//Buffers for send / recv
#define MAX_OUT_MSG_SIZE 256
char input_char;
char b2w_buf_csv[MAX_MSG_SIZE];

#define MAX_RX_MESSAGE_SIZE 16
char w2b_buf[MAX_RX_MESSAGE_SIZE];
uint32_t bufpos = 0;

#define MAX_INT32_DIGITS 12
#define CSV_VALUES 11
char b2wSeparateValues[CSV_VALUES][MAX_INT32_DIGITS];

FIFO_RX_DATATYPE Fifo_RX[FIFO_RX_SIZE];
int8_t Fifo_RX_headptr = 0;
int8_t Fifo_RX_tailptr = 0;

char RX_Fifo_OutString[MAX_MSG_SIZE];

#define NUMBER_OF_TESTS 10

char testingStrings[NUMBER_OF_TESTS][MAX_RX_MESSAGE_SIZE] = {
    "Kp1,100\n",     // Test setting Kp1 to 100
    "Kp2,200\n",     // Test setting Kp2 to 200
    "Ki1,300\n",     // Test setting Ki1 to 300
    "Ki2,400\n",     // Test setting Ki2 to 400
    "Kp1,0\n",       // Test setting Kp1 to 0
    "Kp2,-50\n",     // Test setting Kp2 to -50 (check for handling of negative value if allowed)
    "Ki1,99999\n",   // Test setting Ki1 to a large number
    "Ki2,notnum\n",  // Test setting Ki2 with an invalid number (non-numeric string)
    "Xx1,100\n",     // Test an unknown command (should be ignored or handled)
    "Kp1,10,50\n"    // Test an invalid format with an extra comma
};

uint8_t testingStrings_idx = 0;


/* ================================================== */
/*            FUNCTION PROTOTYPES (DECLARATIONS)      */
/* ================================================== */
bool isValidInteger(const char* buf, uint32_t* out);
uint8_t isDigit(char c);
int strcmp(const char *str1, const char *str2);

/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */
// --------------------------     W2B Parser      ------------------------------
//
// This parser decodes and executes commands from the MQTT Web Appication 
//
void Parser(void) {
  //Perform operations on the w2b buffer to set variables as appropriate
	//atoi() and strtok() may be useful
	
	//char first_token[8];

  //maanav commented this out becasue lib problems, nvrmnd ig it works now, tf
	//strcpy(first_token, strtok(w2b_buf, ","));
	
	#ifdef DEBUG1
		UART_OutString("Token 1: ");
		UART_OutString(first_token);
		UART_OutString("\r\n");
	#endif
}

void DecodeMessage(char msg[MAX_RX_MESSAGE_SIZE]){
  char cmdString[4] = {0};        // Increased to hold 3 characters + null terminator
  int8_t cmdString_idx = 0;

  char numberArray[MAX_RX_MESSAGE_SIZE] = {0};
  int8_t numberArray_idx = 0;

  uint32_t value = 0;
  bool commaFound = false;

  for(int8_t i = 0; i < MAX_RX_MESSAGE_SIZE && msg[i] != RX_TERM_CHAR; i++){  
    if(msg[i] == ','){
      commaFound = true;
    }
    else if(!commaFound && cmdString_idx < 3) {   // Ensure cmdString does not overflow
      cmdString[cmdString_idx++] = msg[i];
    }
    else if (commaFound && numberArray_idx < MAX_RX_MESSAGE_SIZE - 1) { // Avoid overflow
      numberArray[numberArray_idx++] = msg[i];
    } 
  }
  
  // Null-terminate cmdString and numberArray
  cmdString[cmdString_idx] = '\0';
  numberArray[numberArray_idx] = '\0';

  // Serve command
  if(strcmp(cmdString, "Kp1") == 0 && isValidInteger(numberArray, &value)){
    MC_SetKp1(value);
    GPIO_PORTE_DATA_R ^= 0x04;
  } else if(strcmp(cmdString, "Kp2") == 0 && isValidInteger(numberArray, &value)){
    MC_SetKp2(value);
    GPIO_PORTE_DATA_R ^= 0x04;
  } else if(strcmp(cmdString, "Ki1") == 0 && isValidInteger(numberArray, &value)){
    MC_SetKi1(value);
    GPIO_PORTE_DATA_R ^= 0x04;
  } else if(strcmp(cmdString, "Ki2") == 0 && isValidInteger(numberArray, &value)){
    MC_SetKi2(value);
    GPIO_PORTE_DATA_R ^= 0x04;
  }else if(strcmp(cmdString, "X**") == 0 && isValidInteger(numberArray, &value)){
    MC_SetKi2(value);
    GPIO_PORTE_DATA_R ^= 0x04;
  }
}
  
// -----------------------  TM4C_to_MQTT Web App -----------------------------
// This routine publishes clock data to the
// MQTT Web Application via the MQTT Broker
// The data is sent using CSV format:
//
// ----------------------------------------------------------------------------
//    
//    Convert this routine to use a FIFO
//
// 

void TM4C_to_MQTT(void) {

    // Convert Left and Right U values
    intToStr(MC_get_Left_U(), b2wSeparateValues[0]);
    intToStr(MC_get_Right_U(), b2wSeparateValues[1]);

    // Convert Left and Right Error values
    intToStr(MC_get_Left_ERROR(), b2wSeparateValues[2]);
    intToStr(MC_get_Right_ERROR(), b2wSeparateValues[3]);

    // Convert Left and Right RPM values
    intToStr(MC_get_Left_RPMS(), b2wSeparateValues[4]);
    intToStr(MC_get_Right_RPMS(), b2wSeparateValues[5]);

    // Convert KP1, KP2, KI1, and KI2 values
    intToStr(MC_GetKp1(), b2wSeparateValues[6]);
    intToStr(MC_GetKp2(), b2wSeparateValues[7]);
    intToStr(MC_GetKi1(), b2wSeparateValues[8]);
    intToStr(MC_GetKi2(), b2wSeparateValues[9]);

    // Convert Xstar (desired RPMs) value
    intToStr(MC_GetDesiredSpeed(), b2wSeparateValues[10]);

    uint16_t b2w_buf_csv_idx = 0;

    for (int8_t i = 0; i < CSV_VALUES; i++) {
        // Copy each character of b2wSeparateValues[i] into b2w_buf_csv until null terminator
        for (int8_t j = 0; j < MAX_INT32_DIGITS && b2wSeparateValues[i][j] != '\0'; j++) {
            b2w_buf_csv[b2w_buf_csv_idx++] = b2wSeparateValues[i][j];
        }
        
        // Add a comma after each value except the last
        if (i < CSV_VALUES - 1) {
            b2w_buf_csv[b2w_buf_csv_idx++] = ',';
        }
    }
    
    // Null-terminate the CSV string
    b2w_buf_csv[b2w_buf_csv_idx] = ',';
    b2w_buf_csv[b2w_buf_csv_idx+1] = '\n';
    b2w_buf_csv[b2w_buf_csv_idx+2] = '\0';

    UART2_OutString(b2w_buf_csv);
    //UART2_OutString(b2w_buf_csv);
    UART_OutString("tx: ");
    UART_OutString(b2w_buf_csv);

    // Uncomment the following lines if you need to send the CSV via UART
    // UART5_OutString(b2w_buf_csv);

    // #ifdef DEBUG1
    // UART_OutString("B2W: ");
    // UART_OutString(b2w_buf_csv);
    // UART_OutString("\r\n");
    // #endif
}

 
// -------------------------   MQTT_to_TM4C  -----------------------------------
// This routine receives the command data from the MQTT Web App and parses the
// data and feeds the commands to the TM4C.
// -----------------------------------------------------------------------------
//
//    Convert this routine to use a FIFO
//
// 
void MQTT_to_TM4C(void) {
    // Check if there is data in the RX FIFO and UART is not busy
    if ((UART2_FR_R & UART_FR_BUSY) == 0 && (UART2_FR_R & UART_FR_RXFE) == 0) {
        
        char input_char = (UART2_DR_R & 0xFF);   // Read a single byte from the UART

        if (input_char != '\n') {                // If not a newline, process the character
            w2b_buf[bufpos] = input_char;        // Store the character in the buffer
            bufpos++;                            // Advance buffer position
            
            // Optional: Uncomment for debugging received characters
            // UART_OutString("rx: ");
            // UART_OutChar(input_char);
            // UART_OutChar('\n');

        } else {  // End of message (newline received)
            w2b_buf[bufpos] = '\0';              // Null-terminate the string
            Push_Fifo_RX(w2b_buf);               // Push the message to the FIFO

            // Optional: Uncomment to output the message or indicate end of command message
            //UART_OutString("rx: ");
            //UART_OutString(w2b_buf);
            //UART_OutChar('\n');

            bufpos = 0;                          // Reset buffer position for the next message
        }   
    }
}

void PeriodicTask_testUart2(void){
  UART2_OutString(testingStrings[testingStrings_idx]);
  testingStrings_idx++;
}

void PoopTest (void) {
	UART2_OutString("11,11,11,11,11,11,11,11,11,11,11,");
	UART2_OutChar('\n');
	UART_OutString("11,11,11,11,11,11,11,11,11,11,11");
}



void MQTT_Init(void){
  
	//Timer2A_Init(PoopTest, 80000000, 3);
  Timer2A_Init(TM4C_to_MQTT, 80000000, 3);         // Get data every 5ms
  UART2_Init(MQTT_to_TM4C); 
	//Timer2A_Init(PoopTest, 80000000, 3);
  //Timer2A_Init(MQTT_to_TM4C, 400000, 3);         // Get data every 5ms 
  //Timer1A_Init(PeriodicTask_testUart2, 80000000, 3);       // Send data back to MQTT Web App every second
           // Enable Debug Serial Port
  //UART2_DisableRXInterrupt();
   
}

// Push a string into the FIFO
bool Push_Fifo_RX(const FIFO_RX_DATATYPE inString) {
    int8_t next_ptr = (Fifo_RX_headptr + 1 >= FIFO_RX_SIZE) ? 0 : Fifo_RX_headptr + 1;

    if (next_ptr == Fifo_RX_tailptr) {
        return false;  // FIFO is full
    }

    // Copy the string into the FIFO at the head position
    for (uint8_t i = 0; i < MAX_RX_MESSAGE_SIZE - 1; i++) {
        Fifo_RX[Fifo_RX_headptr][i] = inString[i];
        if (inString[i] == '\0') {
            break;  // Stop at null terminator
        }
    }

    Fifo_RX_headptr = next_ptr;  // Update head pointer
    return true;
}

// Pop a string from the FIFO
bool Pop_Fifo_RX(FIFO_RX_DATATYPE outString) {
    if (Fifo_RX_tailptr == Fifo_RX_headptr) {
        return false;  // FIFO is empty
    }

    // Copy the string from the FIFO at the tail position
    for (int i = 0; i < MAX_RX_MESSAGE_SIZE - 1; i++) {
        outString[i] = Fifo_RX[Fifo_RX_tailptr][i];
        if (Fifo_RX[Fifo_RX_tailptr][i] == '\0') {
            break;  // Stop at null terminator
        }
    }

    Fifo_RX_tailptr = (Fifo_RX_tailptr + 1 >= FIFO_RX_SIZE) ? 0 : Fifo_RX_tailptr + 1;  // Update tail pointer in circular fashion
    return true;
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

void ServerCMD(void){
  if(Pop_Fifo_RX(RX_Fifo_OutString)){
      DecodeMessage(RX_Fifo_OutString);
      UART_OutString("rx: ");
      UART_OutString(RX_Fifo_OutString);
      UART_OutChar('\n');
    }
}
