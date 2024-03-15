#include <xc.h>
#include <stdio.h>
#include <math.h>
#include "color.h"
#include "i2c.h"
#include "serial.h" //for debugging

void color_click_init(void){   
    //setup colour sensor via i2c interface
    I2C_2_Master_Init();      //Initialise i2c Master
	color_writetoaddr(0x00, 0x01); //turn on - write 1 to PON bit in device register)
    __delay_ms(3); //need to wait 3ms for everything to start up
	color_writetoaddr(0x00, 0x03); //turn on device ADC
	color_writetoaddr(0x01, 0xFF); //set integration time
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

/* not used anymore
void color_read(struct RGBC_val *p){
    (p->R) = color_read_Red();
    (p->G) = color_read_Green();
    (p->B) = color_read_Blue();
    (p->C) = color_read_Clear();
}
 */

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

char decide_action(){ //reads the colour values and outputs which color it is (numbered 1 - 7)
    
    //----------USED FOR DEBUGGING OVER SERIAL-----------
    /*
    char clear_val[20];
    char red_val[20];
    char green_val[20];
    char blue_val[20];
    char *pclear_val;
    char *pred_val; 
    char *pblue_val;
    char *pgreen_val;
    pclear_val = &clear_val[0];
    pred_val = &red_val[0];
    pgreen_val = &green_val[0];
    pblue_val = &blue_val[0];
    */
    
    float red, green, blue; // define floats to store the color values
    float red_r, green_r, blue_r; //define floats to store ratio of colors
    float rgb_comp[8]; //define array of 8 floats to store comparison values
    char action; //char to assign the action to be done

    //change calibration on color sensor for better reading
    color_writetoaddr(0x01, 0xD5); //set integration time
    color_writetoaddr(0x03, 0xAB); //set wait time (WTIME)
    __delay_ms(200);//let sensor settle
            
    red = color_read_Red();
    green = color_read_Green();
    blue = color_read_Blue();
    
    //reset the sensor values for quick luminosity readings
    color_writetoaddr(0x01, 0xFF); //set integration time
    color_writetoaddr(0x03, 0xFF); //set wait time (WTIME)
    
    //pre-processing of rgb values
    if(red < 666){red = 0;} else{red -= 699;} //remove the red value from black card calibration
    if(green < 446){green = 0;} else{green -= 443;} //remove the green value from black card calibration
    if(blue < 295){blue = 0;} else{blue -= 315;} //remove the blue value from black card calibration
    
    red = red/1621; //divide by the range of red values from calibration
    green = green/1191; //divide by the range of green values from calibration
    blue = blue/885; //divide by the range of blue values from calibration
    
    float total = red + green + blue;
    //find the ratio of the colors
    red_r = red / total;
    green_r = green / total;
    blue_r = blue / total;
    
    //compare ratios of the colors with the measured values from our calibration cards
    rgb_comp[0] = sqrt(pow((red_r - 0.747),2) + pow((green_r - 0.071),2) + pow((blue_r - 0.182),2)); //red
    rgb_comp[1] = sqrt(pow((red_r - 0.147),2) + pow((green_r - 0.489),2) + pow((blue_r - 0.363),2)); //green
    rgb_comp[2] = sqrt(pow((red_r - 0.110),2) + pow((green_r - 0.421),2) + pow((blue_r - 0.469),2)); //blue
    rgb_comp[3] = sqrt(pow((red_r - 0.423),2) + pow((green_r - 0.325),2) + pow((blue_r - 0.252),2)); //yellow
    rgb_comp[4] = sqrt(pow((red_r - 0.431),2) + pow((green_r - 0.263),2) + pow((blue_r - 0.306),2)); //pink
    rgb_comp[5] = sqrt(pow((red_r - 0.664),2) + pow((green_r - 0.137),2) + pow((blue_r - 0.198),2)); //orange
    rgb_comp[6] = sqrt(pow((red_r - 0.271),2) + pow((green_r - 0.366),2) + pow((blue_r - 0.363),2)); //light blue
    rgb_comp[7] = sqrt(pow((red_r - 0.329),2) + pow((green_r - 0.335),2) + pow((blue_r - 0.335),2)); //white

    action = 0; //set initial value for action  
    for(char i = 0; i < 8; i +=1){ //iterate through the comparison array and find the smallest value
        if(rgb_comp[i] < rgb_comp[action]){ 
            action = i;
        }
    }
    
    return action; //return the action - the action numbers correlate to the color positions in the comparison array
}																				


char invert_action(char input_action){
    char output_action;
    switch(input_action){
        case 0:
            output_action = 1;
            break;
        case 1:
            output_action = 0;
            break;
        case 2:
            output_action = 2;
            break;
        case 3:
            output_action = 8;
            break;
        case 4:
            output_action = 9;
            break;
        case 5:
            output_action = 6;
            break;
        case 6:
            output_action = 5;
            break;
        //0-6 are the only possible wall colours (apart from white), so these are the only cases that should occur
        
    }
    return output_action;
}
