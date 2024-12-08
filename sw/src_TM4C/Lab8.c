// Lab8.c
// Runs on RSLK2 with TM4C123 (not LF120)
// 
// Daniel and Jonathan Valvano
// Mark McDermott
// Sept 1, 2024
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
//   PB2  PB12 ERA  T3CCP0
//             ERB  GPIO input  (not used)
//             ELB  GPIO input  (not used)

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
  3 GPIO2         +3.3V
  4 GND     Gnd   GND (70mA)
  5 UTxD    PD6   UART out of ESP8266, 115200 baud
  6 Ch_PD         chip select, 10k resistor to 3.3V
  7 Reset   PB5   TM4C123 can issue output low to cause hardware reset
  8 Vcc           regulated 3.3V supply with at least 70mA
 */
/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdint.h>
#include <stdlib.h>
#include "stdbool.h"
#include "stdio.h"
#include "../inc/tm4c123gh6pm.h"
#include "../inc/Lab8.h"
#include "../inc/Timer5A.h"
#include "../inc/Timer1A.h"
#include "../inc/LaunchPad.h"
#include "../inc/PLL.h"
#include "../inc/MotorControl.h"
#include "../inc/Tachometer.h"
#include "../inc/PIControl.h"
#include "../inc/CLI.h"
#include "../inc/UART.h"

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


/* ================================================== */
/*                    MAIN FUNCTION                   */
/* ================================================== */
int main1(void){
  startupDelay();
  DisableInterrupts();
  PLL_Init(Bus80MHz);    // bus clock at 80 MHz
  PortE_Init();
  LaunchPad_Init();
	UART_Init();
  //Timer5A_Init(PeriodicTask_HeartBeat, HEARTBEAT_PERIOD, 7);
  //CLI_Init();
  //Timer1A_Init(PeriodicTask_UARTOUT, HEARTBEAT_PERIOD, 6);
  MC_init();
  Tachometer_Init();
  PI_Control_Init();
  EnableInterrupts();
	//UART_OutString("hello");
  while(1){
    // if(rpms_dump_buf_idx >= RMPS_DUMP_BUF_SIZE){
    //   //DisableInterrupts();
    //   for(uint32_t i = 0; i < RMPS_DUMP_BUF_SIZE; i++){
    //     if(rpms_dump_buf[i] <= 0){continue;}
    //     intToStr(rpms_dump_buf[i], int_str_buf);
    //     UART_OutString(int_str_buf);
    //     //UART_OutString(",\n");
    //   }
    //   while(1){}
    //}
    //WaitForCMD();
    //UART_InString(int_str_buf, 12);
  }
}
/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */
void PortE_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;//turn on porte
	while((SYSCTL_PRGPIO_R & SYSCTL_RCGCGPIO_R4) == 0){}

	//make pe0-4 gigital outs
	GPIO_PORTE_AMSEL_R &= ~0xFF;
	GPIO_PORTE_AFSEL_R &= ~0xFF;
	GPIO_PORTE_PCTL_R &= ~0xFFFFFFFF;
	GPIO_PORTE_DEN_R |= 0xFF;
	GPIO_PORTE_DIR_R |= 0xFF;

	GPIO_PORTE_DATA_R &= ~0x1F;
}

void PeriodicTask_GetRpms(void){

}

void PeriodicTask_UARTOUT(void){
  //intToStr(MC_GetDesiredSpeed(), int_str_buf);
  //UART_OutString(int_str_buf);
}
void PeriodicTask_HeartBeat(void){
  GPIO_PORTF_DATA_R ^= 0x02;//toggle pe2 for hearbear
}
void startupDelay(void){
  for(uint16_t i = 0; i < UINT16_MAX; i++){}
} 

void intToStr(uint32_t num, char str[12]){
    int i = 0;
    int isNegative = 0;

    // Handle zero case
    if (num == 0) {
        if (i < 11) str[i++] = '0';
        if (i < 11) str[i++] = '\n'; // Add newline character
        str[i] = '\0'; // Null-terminate the string
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        // Handle INT_MIN case (-2147483648)
        if (num == -2147483648) {
            num = 2147483647; // Will adjust after loop
        } else {
            num = -num;
        }
    }

    // Process individual digits
    while (num != 0 && i < 10) { // Leave space for '-', '\n', and '\0'
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    // Adjust for INT_MIN overflow
    if (isNegative && num == 2147483647) {
        if (i < 10) str[0] = '8'; // The last digit of INT_MIN
    }

    // Add minus sign if number is negative
    if (isNegative && i < 11)
        str[i++] = '-';
    // Add newline character if there's space
    if (i < 11)
        str[i++] = '\n';

    str[i] = '\0'; // Append string terminator

    // Reverse the string (excluding '\n' and '\0')
    int start = 0;
    int end = i - 2; // Exclude '\n' and '\0' from reversing
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    str[i-1] = ',';
    str[i] = '\n';
    str[i+1] = '\0';

}

// if(fifofull_flag_cnt == 100){
    //   uint16_t data;
    //   DisableInterrupts();
    //   while(Pop_Period_FIFO_Timer0(&data)){
    //     intToStr(data, int_str_buf);
    //     UART_OutString(int_str_buf);
    //   }
    //   while(1){}
    // }
      // write this