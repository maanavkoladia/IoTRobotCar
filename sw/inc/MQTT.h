#ifndef MQTT_H
#define MQTT_H

/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdbool.h>
#include <stdint.h>
/* ================================================== */
/*            GLOBAL VARIABLE DECLARATIONS            */
/* ================================================== */
#define MAX_MSG_SIZE 256
#define FIFO_RX_SIZE 5
#define MAX_RX_MESSAGE_SIZE 16
typedef char FIFO_RX_DATATYPE[MAX_RX_MESSAGE_SIZE];  // Define FIFO_RX_DATATYPE as an array of chars

/* ================================================== */
/*                 FUNCTION PROTOTYPES                */
/* ================================================== */
// -------------------------   Blynk_to_MQTT  -----------------------------------
//
// This routine receives the MQTT command data from the WEB Application
// via the ESP8266, parses the data and feeds the commands to the TM4C.
//
void MQTT_to_TM4C(void);


// -------------------------- Virtual Pin (VP) Parser --------------------------
//
void Parser(void); 


// -------------------------   MQTT_to_TM4C  -----------------------------------
//
// This routine reads all of the sensor data and sends it back to the MQTT Web Application.
//
void TM4C_to_MQTT(void);

void MQTT_Init(void);

bool Pop_Fifo_RX(FIFO_RX_DATATYPE outString);

bool Push_Fifo_RX(const FIFO_RX_DATATYPE inString);

void ServerCMD(void);

#endif 







