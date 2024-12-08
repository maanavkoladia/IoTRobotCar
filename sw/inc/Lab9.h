#ifndef LAB9_H
#define LAB9_H

/* ================================================== */
/*                      INCLUDES                      */
/* ================================================== */

/* ================================================== */
/*            GLOBAL VARIABLE DECLARATIONS            */
/* ================================================== */

/* ================================================== */
/*                 FUNCTION PROTOTYPES                */
/* ================================================== */
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // Go into low power mode

void intToStr(int32_t num, char str[12]);

#endif 

