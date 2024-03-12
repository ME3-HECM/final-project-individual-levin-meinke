// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#include <xc.h>
#include <stdio.h>
#include "ADC.h"
#include "color.h"
#include "dc_motor.h"
#include "i2c.h"
#include "interrupts.h"
#include "serial.h"
#include "timers.h"

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  

void main(void){
    //init all inits for used modules
    Timer0_init(); //timer
    I2C_2_Master_Init(); //i2c coms
    color_click_init(); //color click module
    initDCmotorsPWM(99); //dc motors pwm
    initUSART4(); //init USART if needed for debugging via serial
  
    //turn on color click LEDs
    color_click_toggleLED();
    
	//???don't forget TRIS for your output!
    
    //declare structures for left and right motors
    struct DC_motor motorL, motorR; 		//declare two DC_motor structures
    unsigned int PWMcycle = 99;
    //left
    motorL.power=0; 						//zero power to start
    motorL.direction=1; 					//set default motor direction
    motorL.brakemode=1;						// brake mode (slow decay)
    motorL.posDutyHighByte=(unsigned char *)(&CCPR1H);  //store address of CCP1 duty high byte
    motorL.negDutyHighByte=(unsigned char *)(&CCPR2H);  //store address of CCP2 duty high byte
    motorL.PWMperiod=PWMcycle; 			//store PWMperiod for motor (value of T2PR in this case)
    //right
    motorR.power=0; 						//zero power to start
    motorR.direction=1; 					//set default motor direction
    motorR.brakemode=1;						// brake mode (slow decay)
    motorR.posDutyHighByte=(unsigned char *)(&CCPR3H);  //store address of CCP3 duty high byte
    motorR.negDutyHighByte=(unsigned char *)(&CCPR4H);  //store address of CCP4 duty high byte
    motorR.PWMperiod=PWMcycle; 			//store PWMperiod for motor (value of T2PR in this case)
    
    char *pmL; //declare pointer motor left
    char *pmR; //declare pointer motor right
    pmL = &motorL; // assign pmL to the address of motorL
    pmR = &motorR; // assign pmR to the address of motorR
    
    char red_val[20];
    char *pred_val; 
    pred_val = &red_val[0];

    while(1){
        //read red value every second and send to serial
        sprintf(red_val,"red = %d \r\n",color_read_Red());
        sendStringSerial4(pred_val);
        __delay_ms(1000);
    }
}
