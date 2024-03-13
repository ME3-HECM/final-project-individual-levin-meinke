#include <xc.h>
#include "interrupts.h"
#include "ADC.h"
#include "serial.h"

// Function to turn on interrupts
void Interrupts_init(void)
{
    INTCONbits.PEIE=1; //enables peripheral interrupts in the INTCON reigster 
    PIE2bits.C1IE=1; //enable peripheral interrupt C1IE in PIE2 of INTCON register for comparator flagging
    //set up interrupts from the color click
    //TRISBbits.TRISB0 = 1; //pin config for b0 -> disable output driver
    //INTCONbits.INT0IE=1; //how do we enable the interrupt?
    PIE0bits.IOCIE=1; //enable interrupt on change register (RB0)
    INTCONbits.GIE=1; 	//turn on interrupts globally
    
    //PIE0bits.TMR0IE=1; //enable timer overflow bit - breaks??
    //PIE4bits.RC4IE=1;	//receive interrupt
    //PIE4bits.TX4IE=1;	//transmit interrupt (do not turn on in your init routine, we will turn on using a function when we want to send something)
}

// High priority interrupt service routines
void __interrupt(high_priority) HighISR()
{
    // Comparator flag, raised when ambient light intensity goes above or below "dawn/dusk" boundary value
	//if(PIR2bits.C1IF){
    //    LATHbits.LATH3 = !LATHbits.LATH3; // Toggles LED
        
        //CODE TO EXECUTE   
        
    //    PIR2bits.C1IF=0; // Clear the interrupt flag
    //}
    
    // Timer flag, raised every minute in standard mode / 1/60s in testing mode
    //if (PIR0bits.TMR0IF) {
    //    //CODE TO EXECUTE
        // Sets timer initial value to 3035 for perfect overflow accuracy, writes High register first
    //    TMR0H=0b00001011;
    //    TMR0L=0b11011011; 
        
    //    PIR0bits.TMR0IF=0; // Clear the interrupt flag
    //}
    //serial com receive interrupt
    //if (PIR4bits.RC4IF) {
    //    return RC4REG;
    //}  
    //interrupt from pin RBO (color click interrupt)
    if (PORTBbits.RB0){
        LATHbits.LATH3 = 0;
        sendCharSerial4('a');
        //color_writetoaddr(char address, 0x06); //reset the interrupt in COMMAND register
        //inline the function?
    }
    
}

