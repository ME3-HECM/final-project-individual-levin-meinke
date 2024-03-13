#include <xc.h>
#include "color.h"
#include "i2c.h"

void color_click_init(void){   
    //setup colour sensor via i2c interface
    I2C_2_Master_Init();      //Initialise i2c Master
	color_writetoaddr(0x00, 0x01); //turn on - write 1 to PON bit in device register)
    __delay_ms(3); //need to wait 3ms for everything to start up
	color_writetoaddr(0x00, 0x03); //turn on device ADC
	color_writetoaddr(0x01, 0xD5); //set integration time
    color_writetoaddr(0x03, 0xFF); //set wait time (WTIME)
    
    //set-up interrupts to stop the buggy when it approaches a color card
    //color_writetoaddr(0x00, 0x13);//enable interrupt
    //color_writetoaddr(0x04, 0x00); //set lower threshold, lower byte
    //color_writetoaddr(0x05, 0x00); //set lower threshold, upper byte
    //color_writetoaddr(0x06, 0x00); //set upper threshold, lower byte
    //color_writetoaddr(0x07, 0x07); //set upper threshold, upper byte 7 threshold of 1792
    //color_writetoaddr(0x0C, 0x03); //set interrupt persistance filter to 3 consecutive readings
    //note: clear interrupt using COMMAND
    
    //set initial value for LEDS on the color click to off (toggle with toggle function)
    LATGbits.LATG0=0; //red LED
    LATAbits.LATA3=0; //blue LED
    LATEbits.LATE7=0; //green LED
    
    //set tris values for pins to output corresponding to the LEDs on color click
    TRISGbits.TRISG0=0;
    TRISAbits.TRISA3=0;
    TRISEbits.TRISE7=0;
    
}

void color_writetoaddr(char address, char value){
    I2C_2_Master_Start();         //Start condition
    I2C_2_Master_Write(0x52 | 0x00);     //7 bit device address + Write mode
    I2C_2_Master_Write(0x80 | address);    //command + register address
    I2C_2_Master_Write(value);    
    I2C_2_Master_Stop();          //Stop condition
}


void color_read(struct RGBC_val *p){
    (p->R) = color_read_Red();
    (p->G) = color_read_Green();
    (p->B) = color_read_Blue();
    (p->C) = color_read_Clear();
}

unsigned int color_read_Clear(void){
	unsigned int tmp;
	I2C_2_Master_Start();         //Start condition
	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x14);    //command (auto-increment protocol transaction) + start at RED low register
	I2C_2_Master_RepStart();			// start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
	tmp=I2C_2_Master_Read(1);			//read the Red LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8); //read the Red MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();          //Stop condition
	return tmp;
}

unsigned int color_read_Red(void){
	unsigned int tmp;
	I2C_2_Master_Start();         //Start condition
	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x16);    //command (auto-increment protocol transaction) + start at RED low register
	I2C_2_Master_RepStart();			// start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
	tmp=I2C_2_Master_Read(1);			//read the Red LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8); //read the Red MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();          //Stop condition
	return tmp;
}

unsigned int color_read_Green(void){
	unsigned int tmp;
	I2C_2_Master_Start();         //Start condition
	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x18);    //command (auto-increment protocol transaction) + start at RED low register
	I2C_2_Master_RepStart();			// start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
	tmp=I2C_2_Master_Read(1);			//read the Green LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8); //read the Green MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();          //Stop condition
	return tmp;
}

unsigned int color_read_Blue(void){
	unsigned int tmp;
	I2C_2_Master_Start();         //Start condition
	I2C_2_Master_Write(0x52 | 0x00);     //7 bit address + Write mode
	I2C_2_Master_Write(0xA0 | 0x1A);    //command (auto-increment protocol transaction) + start at RED low register
	I2C_2_Master_RepStart();			// start a repeated transmission
	I2C_2_Master_Write(0x52 | 0x01);     //7 bit address + Read (1) mode
	tmp=I2C_2_Master_Read(1);			//read the Green LSB
	tmp=tmp | (I2C_2_Master_Read(0)<<8); //read the Green MSB (don't acknowledge as this is the last read)
	I2C_2_Master_Stop();          //Stop condition
	return tmp;
}



//function to toggle LEDs on the color click
void color_click_toggleLED(void){
    LATGbits.LATG0 = !LATGbits.LATG0;
    LATAbits.LATA3 = !LATAbits.LATA3;
    LATEbits.LATE7 = !LATEbits.LATE7;
}

