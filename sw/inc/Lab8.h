#ifndef LAB8_H
#define LAB8_H

/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */

/* ================================================== */
/*            GLOBAL VARIABLE DECLARATIONS            */
/* ================================================== */
extern uint32_t fifofull_flag_cnt;
extern char int_str_buf[12];
/* ================================================== */
/*                 FUNCTION PROTOTYPES                */
/* ================================================== */
void DisableInterrupts(void);           // Disable interrupts
void EnableInterrupts(void);            // Enable interrupts
void WaitForInterrupt(void);

void intToStr(uint32_t num, char str[12]);
#endif 

