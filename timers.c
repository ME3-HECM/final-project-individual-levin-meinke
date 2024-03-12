#include <xc.h>
#include "timers.h"

/************************************
 * Function to set up timer 0
************************************/
void Timer0_init(void)
{
    T0CON1bits.T0CS=0b010; // Fosc/4, setting clock source to 16Mz
    T0CON1bits.T0ASYNC=1; // See datasheet errata - needed to ensure correct operation when Fosc/4 used as clock source
    T0CON1bits.T0CKPS=0b1000; // 1:256
    T0CON0bits.T0OUTPS=0b000; // 1:1
    
    T0CON0bits.T016BIT=1; // 16-bit mode	
        
    // Sets timer initial value to 3035 for perfect overflow accuracy, writes High register first
    TMR0H=0b00001011;
    TMR0L=0b11011011;
    
    T0CON0bits.T0EN=1; // Start the timer
}

/************************************
 * Function to return the full 16bit timer value
 * Note TMR0L and TMR0H must be read in the correct order, or TMR0H will not contain the correct value
************************************/
unsigned int get16bitTMR0val(void){
	//add your code to get the full 16 bit timer value here
}

