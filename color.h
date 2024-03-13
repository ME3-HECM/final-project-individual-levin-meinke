#ifndef _color_H
#define _color_H

#include <xc.h>

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  

struct RGBC_val{
    unsigned int R; //red
    unsigned int G; //green
    unsigned int B; //blue
    unsigned int C; //clear
};

/********************************************//**
 *  Function to initialise the colour click module using I2C
 ***********************************************/
void color_click_init(void);

/********************************************//**
 *  Function to write to the colour click module
 *  address is the register within the colour click to write to
 *	value is the value that will be written to that address
 ***********************************************/
void color_writetoaddr(char address, char value);

//functions to read clear red, green, blue and clear values
//Returns a 16 bit ADC value representing colour intensity
unsigned int color_read_Clear(void);
unsigned int color_read_Red(void);
unsigned int color_read_Green(void);
unsigned int color_read_Blue(void);

//function to read all colors and store in struct RGBC_val
void color_read(struct RGBC_val *p);

//function to turn LEDs on and off
void color_click_toggleLED(void);

#endif
