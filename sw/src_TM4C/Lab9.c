// Lab9.c
// Runs on TM4C123 (not LF120)
// 
// Daniel and Jonathan Valvano
// 
// Mark McDermott
// Add: MQTT.c ESP8266.c
// July 1, 2024
// Bump switches
//   TM4C MSPM0
//   PA5  PA27    Left, Bump 0,
//   PF0  PB15    Center Left, Bump 1,
//   PB3  PA28    Center Right, Bump 2
//   PC4  PA31    Right, Bump 3

// Motor
//   TM4C MSPM0
//   PF2  PB4  Motor_PWML, ML+, IN3, PWM M1PWM6
//   PF3  PB1  Motor_PWMR, MR+, IN1, PWM M1PWM7
//   PA3  PB0  Motor_DIR_L,ML-, IN4, GPIO 0 means forward, 1 means backward
//   PA2  PB16 Motor_DIR_R,MR-, IN2, GPIO 0 means forward, 1 means backward

// tachometer
//   TM4C MSPM0
//   PB7  PB8  ELA  T0CCP1
//   PB6  PB7  ERB  GPIO input 
//   PA4  PB6  ELB  GPIO input 
//   PB2  PB12 ERA  T3CCP0

// SSD1306 I2C OLED
//   TM4C MSPM0
//   PA6  PB2  SCL I2C clock
//   PA7  PB3  SDA I2C data

// IR analog distance sensors
//   TM4C MSPM0
//   PE4  PA26 Right  Ain9
//   PE5  PB24 Center Ain8 
//   PE1  PA24 Left   Ain2

// TF Luna TOF distance sensor
//   TM4C MSPM0
//   PB1  PA8 TxD microcontroller sensor RxD pin 2
//   PB0  PA9 RxD microcontroller sensor TxD pin 3

// LaunchPad pins
//   PF1 red LED 
//   PF2 blue LED 
//   PF3 green LED 
//   PF4 S2 negative logic switch
//   PF0 S1 negative logic switch (shared with Bump 1)

/* Hardware connections
  Vcc should be connected to RSLK 3.3V (not debugger)
 /------------------------------\
 |              chip      1   8 |
 | Ant                    2   7 |
 | enna       processor   3   6 |
 |                        4   5 |
 \------------------------------/ 
 Set #define  for UART2 (PD) and Reset PB5 
 
 ESP8266    TM4C123 
  1 URxD    PD7   UART out of TM4C123, 115200 baud
  2 GPIO0         +3.3V for normal operation (ground to flash)
  3 GPIO2         Status
  4 GND     Gnd   GND (70mA)
  5 UTxD    PD6   UART out of ESP8266, 115200 baud
  6 Ch_PD         chip select, 10k resistor to 3.3V
  7 Reset   PB5   TM4C123 can issue output low to cause hardware reset
  8 Vcc           regulated 3.3V supply with at least 70mA
 */
 
 



//----- Prototypes of functions in startup.s  ----------------------
//


// -----------------------------------------------------------------
// -------------------- MAIN LOOP ----------------------------------
//


/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdio.h>
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/Lab9.h"
#include "../inc/PLL.h"
#include "../inc/Timer5A.h"
#include "../inc/UART.h"
#include "../inc/UART2.h"
#include "../inc/UART5.h"
#include "../inc/esp8266.h"
#include "../inc/MQTT.h"
#include "../inc/Tachometer.h"
#include "../inc/PIControl.h" 
#include "../inc/MotorControl.h"
#include "../inc/CLI.h"

/* ================================================== */
/*            GLOBAL VARIABLE DEFINITIONS             */
/* ================================================== */
#define HEARTBEAT_PERIOD 80000000 //1 sec

uint32_t fifofull_flag_cnt = 0;

char int_str_buf[12];
/* ================================================== */
/*            FUNCTION PROTOTYPES (DECLARATIONS)      */
/* ================================================== */
void startupDelay(void);
void PeriodicTask_HeartBeat(void);
void PeriodicTask_UARTOUT(void);
void PortE_Init(void);

char cmdstring[MAX_MSG_SIZE];
/* ================================================== */
/*                    MAIN FUNCTION                   */
/* ================================================== */
int main(void){    
  startupDelay();   
  DisableInterrupts();        // Disable interrupts until finished with inits
  PLL_Init(Bus80MHz);         // Bus clock at 80 MHz

	UART_Init();                // Enable Debug Serial Port
	//UART5_Init();
	//UART2_Init(PeriodicTask_HeartBeat);
  
  UART_OutString("starting init ritual\n");
  MQTT_Init();
  
  PortB_Init();
  Reset_8266();               // Reset the WiFi module
  SetupWiFi();                // Setup communications to MQTT Broker via 8266 WiFi
  PortE_Init();
  Timer5A_Init(PeriodicTask_HeartBeat, HEARTBEAT_PERIOD, 7);

  
	
  
  MC_init();
  //Tachometer_Init();
  //PI_Control_Init();
  //CLI_Init();
  EnableInterrupts();
  
  while(1){  
    ServerCMD();
  }
}

/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */

void PortE_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;//turn on porte
	while((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R4) == 0){}

	//make pe0-4 gigital outs
	GPIO_PORTE_AMSEL_R &= ~0x0F;
	GPIO_PORTE_AFSEL_R &= ~0x0F;
	GPIO_PORTE_PCTL_R &= ~0x0000FFFF;
	GPIO_PORTE_DEN_R |= 0x0F;
	GPIO_PORTE_DIR_R |= 0x0F;

	GPIO_PORTE_DATA_R &= ~0x1F;
}

void PeriodicTask_GetRpms(void){

}

void PeriodicTask_UARTOUT(void){
  //intToStr(MC_GetDesiredSpeed(), int_str_buf);
  //UART_OutString(int_str_buf);
}
void PeriodicTask_HeartBeat(void){
  //UART_OutString("thump\n");
  GPIO_PORTE_DATA_R ^= 0x02;//toggle pe1 for hearbear
}
void startupDelay(void){
  for(uint16_t i = 0; i < UINT16_MAX; i++){}
} 

void intToStr(int32_t num, char str[12]) {
    int isNegative = 0;
    int index = 0;

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num; // Make the number positive for conversion
    }

    // Convert integer to string in reverse order
    do {
        str[index++] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    // Add negative sign if needed
    if (isNegative) {
        str[index++] = '-';
    }

    // Null-terminate the string
    str[index] = '\0';

    // Reverse the string to get the correct order
    int start = 0;
    int end = index - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}
