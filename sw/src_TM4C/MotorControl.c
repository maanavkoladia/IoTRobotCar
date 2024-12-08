// ----------------------------------------------------------------------
//
// MotorControl.c
// 
// ----------------------------------------------------------------------
// 
//  This code runs the Motor Controller 
//  Mark McDermott  June 21, 2018
//  Jonathan Valvano July 1, 2024
//  It is derived from:
//        PI Motor Controller.
//        Daniel Valvano and Jonathan Valvano
//        June 5, 2014
//
// **********   Lab solution, do not post  ******************8
//
// Two DC motor with L293 driver
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
/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */
#include <stdint.h>
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/MotorControl.h"
#include "../inc/Tachometer.h"
#include "../inc/Lab8.h"
#include "../inc/UART.h"
//#include "Tachometer.c"

/* ================================================== */
/*            GLOBAL VARIABLE DEFINITIONS             */
/* ================================================== */
#define PA2             (*((volatile uint32_t *)0x40004010))
#define PA3             (*((volatile uint32_t *)0x40004020))

#define DIR_L 8
#define DIR_R 4

#define MAXSPEED 1500 //max allowable motor speed in rpm
#define MINSPEED 0
#define MOTORMIN 2
#define MOTORMAX (MOTOR_PERIOD-2)

int32_t Left_U;
int32_t Right_U;
int32_t Left_ERROR;
int32_t Right_ERROR;
int32_t Left_RPMS;
int32_t Right_RPMS;

int32_t desired_speed=0;
int32_t KP1=0;//num
int32_t KP2=0;//den
int32_t KI1=0;//num
int32_t KI2=0;//den


/* ================================================== */
/*            FUNCTION PROTOTYPES (DECLARATIONS)      */
/* ================================================== */


/* ================================================== */
/*                 FUNCTION DEFINITIONS               */
/* ================================================== */
void PWM_Init(void){
  SYSCTL_RCGCPWM_R |= 0x02;         // 1) activate PWM1
  SYSCTL_RCGCGPIO_R |= 0x00000021;  // activate clock for Port A,F
  while((SYSCTL_PRGPIO_R&0x21) != 0x21){};// allow time for clock to stabilize
  GPIO_PORTA_DIR_R |= 0x0C;        // PA3 PA2 outputs
  GPIO_PORTA_DEN_R |= 0x0C;        // enable digital I/O on PA3 PA2
  GPIO_PORTA_DATA_R &= ~0x0C;
  PA2 = PA3 = 0;  // forward
  GPIO_PORTF_AFSEL_R |= 0x0C;     // enable alt funct on PF3 PF2
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00FF)|0x00005500;
    // PF2 is M1PWM6
    // PF3 is M1PWM7
  GPIO_PORTF_DIR_R |= 0x0C;             // PF3 PF2 output
  GPIO_PORTF_DEN_R |= 0x0C;             // enable digital I/O on PF3 PF2
  GPIO_PORTF_DR8R_R |= 0x0C;            // high current output on PF3 PF2
    // bit 20=1 for USEPWMDIV
    // bits 19-17 = 0 for divide by 2
    // bits 19-17 = n for divide by 2*(2^n)
    // bits 19-17 = 3 for divide by 16
    // bits 19-17 = 5 for divide by 64
  SYSCTL_RCC_R = 0x00120000 |           // 3) use PWM divider 
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /4 divider
  PWM1_3_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM1_3_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PF2 goes low on LOAD
  // PF2 goes high on CMPA down
  PWM1_3_LOAD_R = MOTOR_PERIOD - 1;    // 5) cycles needed to count down to 0
  PWM1_3_CMPA_R = 10000;                  // 6) count value when output rises
    
  PWM1_3_GENB_R = 0xC08;               // low on LOAD, high on CMPB down
  // PF3 goes low on LOAD
  // PF3 goes high on CMPB down  
  PWM1_3_CMPB_R = 10000;                   // 6) count value when output rises
  PWM1_3_CTL_R |= 0x00000001;           // 7) start PWM1
  PWM1_ENABLE_R |= 0x000000C0;          // enable PF2 is M1PWM6 and PF3 is M1PWM7
}


/**
 * Initialize RSLK Controller
 * @param none
 * @return none
 * @brief  Initialize tach input and motor outputs
 * @note period is 40000 us
 */
void MC_init(void){
  //PWM_Init();
  Left_ERROR = 300;
  Right_ERROR = 4000;
  Left_U = -10;
  Right_U = 100;
  Left_RPMS = 1000;
  Right_RPMS = 1000;
  MC_SetKp1(1200);
  MC_SetKp2(10000);
  MC_SetKi1(200);
  MC_SetKi2(10000);
  MC_SetDesiredSpeed(400);
}

int32_t MC_get_Left_U() {
    return Left_U;
}

int32_t MC_get_Right_U() {
    return Right_U;
}

int32_t MC_get_Left_ERROR() {
    return Left_ERROR;
}

int32_t MC_get_Right_ERROR() {
    return Right_ERROR;
}

int32_t MC_get_Left_RPMS() {
    return Left_RPMS;
}

int32_t MC_get_Right_RPMS() {
    return Right_RPMS;
}

/*
 * @param newSpeed
 * @return desiredSpeed
 * @brief  Get desired speed
 */
int32_t MC_GetDesiredSpeed(void){
	return desired_speed;
}

/**
 * Set desired speed in 0.1 rpm.
 * Robot should move in a straight line
 * @param newSpeed
 * @return none
 * @brief  Set desired speed
 */
void MC_SetDesiredSpeed(int32_t newSpeed){
	SpeedFlagR = 0;
	SpeedFlagL = 0;
  if(newSpeed <= MAXSPEED && newSpeed >= MINSPEED){
    desired_speed = newSpeed;
		ClearCounts();
  }
}

/**
 * Set Kp1
 * @param kp1
 * @return none
 * @brief  Set numerator for proportial control
 */
void MC_SetKp1(int32_t kp1){
	KP1=kp1;
}

/**
 * Set Kp2
 * @param kp2
 * @return none
 * @brief  Set denominator for proportial control
 */
void MC_SetKp2(int32_t kp2){
	KP2=kp2;
}

/**
 * Set Ki1
 * @param ki1
 * @return none
 * @brief  Set numerator for integral control
 */
void MC_SetKi1(int32_t ki1){
	KI1=ki1;
}

/**
 * Set Ki2
 * @param ki2
 * @return none
 * @brief  Set denominator for integral control
 */
void MC_SetKi2(int32_t ki2){
	KI2=ki2;
}

/**
 * Get Kp1
 * @param none
 * @return Kp1
 * @brief  Get numerator for proportial control
 */
int32_t MC_GetKp1(void){
	return KP1;
}

/**
 * Get Kp2
 * @param none
 * @return Kp2
 * @brief  Get denominator for proportial control
 */
int32_t MC_GetKp2(void){
	return KP2;
}

/**
 * Get Ki1
 * @param none
 * @return Ki1
 * @brief  Get numerator for integral control
 */
int32_t MC_GetKi1(void){
	return KI1;
}

/**
 * Get Ki2
 * @param none
 * @return Ki2
 * @brief  Get denominator for integral control
 */
int32_t MC_GetKi2(void){
	return KI2;
}

/**
 * Get LeftE
 * @param none
 * @return LeftE
 * @brief  Get controller error on left
 */
int32_t MC_GetLeftE(void){
    Left_RPMS = (int32_t)Tachometer_GetLeftrpm();
    int32_t rmpDifference = desired_speed - Left_RPMS;
    return ((rmpDifference * 2227) >> 8) -1140;
  //return Tachometer_GetLeftrpm() - desired_speed;
}

/**
 * Get RightE
 * @param none
 * @return RightE
 * @brief  Get controller error on right
 */
int32_t MC_GetRightE(void){
  Right_RPMS = (int32_t)Tachometer_GetRightrpm();
	int32_t rpmDifference = desired_speed - Right_RPMS; 
  return ((rpmDifference * 2227) >> 8) -1140;
  //return Tachometer_GetRightrpm() - desired_speed;
}

/**
 * Get LeftU
 * @param none
 * @return LeftU
 * @brief  Get controller actuator output on left
 */
uint32_t MC_GetLeftU(void){
    int32_t LeftE = MC_GetLeftE();
    Left_ERROR = LeftE;
    static int32_t LeftI = 0; // Static variable to retain value between calls
    int32_t LeftP = (KP1 * LeftE) / KP2; // Proportional term
    LeftI = LeftI + (KI1 * LeftE) / KI2; // Integral term accumulation

    // Anti-windup: Limit the integral term to prevent excessive accumulation
    if (LeftI < (MOTORMIN - LeftP)) LeftI = MOTORMIN - LeftP;
    if (LeftI > (MOTORMAX - LeftP)) LeftI = MOTORMAX - LeftP;

    int32_t LeftU = LeftP + LeftI; // Total control output

    // Limit the control output to valid PWM duty cycle range
    if (LeftU < MOTORMIN) LeftU = MOTORMIN;
    if (LeftU > MOTORMAX) LeftU = MOTORMAX;
    Left_U = LeftU;
    return (uint32_t) LeftU;
}


/**
 * Get RightU
 * @param none
 * @return RightU
 * @brief  Get controller actuator output on right
 */
uint32_t MC_GetRightU(void){
    int32_t RightE = MC_GetRightE();
    Right_ERROR = RightE;
    static int32_t RightI = 0; // Static variable to retain value between calls
    int32_t RightP = (KP1 * RightE) / KP2; // Proportional term
    RightI = RightI + (KI1 * RightE) / KI2; // Integral term accumulation

    // Anti-windup: Limit the integral term to prevent excessive accumulation
    if (RightI < (MOTORMIN - RightP)) RightI = MOTORMIN - RightP;
    if (RightI > (MOTORMAX - RightP)) RightI = MOTORMAX - RightP;

    int32_t RightU = RightP + RightI; // Total control output

    // Limit the control output to valid PWM duty cycle range
    if (RightU < MOTORMIN) RightU = MOTORMIN;
    if (RightU > MOTORMAX) RightU = MOTORMAX;
    Right_U = RightU;
    return (uint32_t) RightU;
}


/**
 * Get Time 
 * since last change in desired speed, 10 ms
 * @param none
 * @return Time
 * @brief  Get controller time
 * @note Controller runs at 100Hz
 */
 uint32_t MC_Time(void){
	 return 0;
 }
 
// get dump array pointers
// left and right speeds are dumped after each change in desired speed
void MC_DumpSpeed(uint32_t *leftspeed, uint32_t *rightspeed){
	//return 0;
	return;
}

uint32_t *MC_DumpRight(void){
	return 0;
}

uint32_t *MC_DumpLeft(void){
	return 0;
}

void SetDutyCycles(uint32_t dutyRight, uint32_t dutyLeft){
    PWM1_3_CMPA_R = dutyLeft;
    PWM1_3_CMPB_R = dutyRight;
		
}