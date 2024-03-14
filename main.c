// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
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
    Interrupts_init(); //interrupts
    I2C_2_Master_Init(); //i2c coms
    color_click_init(); //color click module
    initDCmotorsPWM(99); //dc motors pwm
    initUSART4(); //init USART if needed for debugging via serial
  
    //turn on color click LEDs
    color_click_toggleLED();
    
    //enable led
    LATHbits.LATH3=1; //set initial output state for RH3
    TRISHbits.TRISH3=0; //set TRIS value for pin (output)
    
    //enable the ON button (RF2)
    TRISFbits.TRISF2=1; //set TRIS value for pin (input)
    ANSELFbits.ANSELF2=0; //turn off analogue input on pin
    
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
    
    //create pointers for motorL and motorR
    struct DC_motor *pmL= &motorL;  
    struct DC_motor *pmR= &motorR;
    
    //----------USED FOR DEBUGGING OVER SERIAL-----------
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
    
    unsigned int lum; //luminosity reading to detect wall/color card
    unsigned int redm, greenm, bluem; //readings for red, green & blue values (in main code)
    char action; //action to be done
    bool going_forward = false; //boolean whether buggy is moving forward
    unsigned int previously_measured_time, measured_time; 
    int action_to_do;
    unsigned int timings[20]; //record the timings of going forwards
    int actions[20]; //record the actions in order of when they happened
    int actions_completed = 0;
    
    // ------------ instruction codes -----------------
    // turn right 90: 0
    // turn left 90: 1
    // turn 180: 2
    // reverse one square and turn right 90: 3
    // reverse one square and turn left 90: 4
    // turn right 135:  5
    // turn left 135: 6
    // turn around and retrace: 7

    
    __delay_ms(2000); //delay to allow operator to turn on buggy
    
    //----------------- testing stopping & color readings --------------------
    
    
    while(1){
        turn_left_135(pmL, pmR);
        __delay_ms(1000);
    }
    // test loop for color calibration
    /*
    while(1){
        
        color_writetoaddr(0x01, 0xD5); //set integration time
        color_writetoaddr(0x03, 0xAB); //set wait time (WTIME)
        __delay_ms(200);//let sensor adjust settings
        
        redm = color_read_Red();
        greenm = color_read_Green();
        bluem = color_read_Blue();
            
        sprintf(red_val,"mred = %d \r\n",redm);
        sendStringSerial4(pred_val);
        sprintf(green_val,"mgreen = %d \r\n",greenm);
        sendStringSerial4(pgreen_val);
        sprintf(blue_val,"mblue = %d \r\n",bluem);
        sendStringSerial4(pblue_val);
            
        action = decide_action(redm, greenm, bluem);
            
        sprintf(clear_val,"action = %d \r\n",action);
        sendStringSerial4(pclear_val);
            
        //reset sensor vals
        color_writetoaddr(0x01, 0xFF); //set integration time
        color_writetoaddr(0x03, 0xFF); //set wait time (WTIME)

        __delay_ms(3000);
        __delay_ms(3000);
        __delay_ms(3000);
    }
    */
    
    while(1){
        if(!going_forward){
            // if in here : started program or just finished action.
            // time to reset timer and start moving forward
            resetTimer0();
            fullSpeedAhead(pmL, pmR);
            going_forward = true;
        }
        // read the clear sensor
        lum = color_read_Clear();
        //if above threshold you want to stop and find card color
        if (lum > 30){
            //stop buggy
            measured_time = get16bitTMR0val(); //measure time going forward
            stop(pmL, pmR);
            //say you've stopped going forward to satisfy the if statement above later
            going_forward = false;
            __delay_ms(100); //wait for buggy to stop
            
            // sprintf(red_val,"action = %d \r\n",decide_action());
            // sendStringSerial4(pred_val);
            

            //this gets your action (but make sure you declare red, green, blue, action)
            color_writetoaddr(0x01, 0xD5); //set integration time
            color_writetoaddr(0x03, 0xAB); //set wait time (WTIME)
            __delay_ms(200);//let sensor adjust settings
            
            redm = color_read_Red();
            greenm = color_read_Green();
            bluem = color_read_Blue();
            
            sprintf(red_val,"red = %d \r\n",redm);
            sendStringSerial4(pred_val);
            sprintf(green_val,"green = %d \r\n",greenm);
            sendStringSerial4(pgreen_val);
            sprintf(blue_val,"bluee = %d \r\n",bluem);
            sendStringSerial4(pblue_val);
            
            action = decide_action(redm, greenm, bluem);
            
            sprintf(clear_val,"action = %d \r\n",action);
            sendStringSerial4(pclear_val);
            
            //reset sensor vals
            color_writetoaddr(0x01, 0xFF); //set integration time
            color_writetoaddr(0x03, 0xFF); //set wait time (WTIME)

            __delay_ms(3000);
            __delay_ms(3000);
            __delay_ms(3000);
            
        }    
    }
    

    //main loop 
    while(1){
        if(!going_forward){
            // if in here : started program or just finished action.
            // time to reset timer and start moving forward
            resetTimer0();
            fullSpeedAhead(pmL, pmR);
            going_forward = true;
        }
        // read the colour sensor
        lum = color_read_Clear();
        //if above 1500 you want to stop and find color
        if (lum > 1500){
            //stop buggy
            measured_time = get16bitTMR0val(); //measure time going forward
            stop(pmL, pmR);
            __delay_ms(250); //wait for buggy to settle

            //say you've stopped going forward to satisfy the if statement above later
            going_forward = false;

            // store the measured time so we can use it later in retracing
            timings[actions_completed] = measured_time;
            //decide what colour it is
            
            color_writetoaddr(0x01, 0xD5); //set integration time
            color_writetoaddr(0x03, 0xAB); //set wait time (WTIME)
            __delay_ms(200);//let sensor adjust settings
            
            redm = color_read_Red();
            greenm = color_read_Green();
            bluem = color_read_Blue();
            
            action_to_do = decide_action(redm, greenm, bluem);
            
            //store the action to do so you can use it later when retracing
            actions[actions_completed] = action_to_do;

            // we have stored all data from this action. We now can increment 'actions completed'
            actions_completed += 1;

            //give the robot a command based on the actions its supposed to do
            if(action_to_do == 0){ //turn right 90
                reverse_after_read(pmL, pmR);
                turn_right_90(pmL, pmR);
            }
            else if(action_to_do == 1){ //turn left
                reverse_after_read(pmL, pmR);
                turn_left_90(pmL, pmR);
            }
            else if(action_to_do == 2){ //180 turn -> turn right twice (less calibration error)
                reverse_after_read(pmL, pmR);
                turn_right_90(pmL, pmR);
                turn_right_90(pmL, pmR);
            }
            else if(action_to_do == 3){ //reverse one square and then turn right 90
                reverse_after_read(pmL, pmR);
                reverse_one_square(pmL, pmR);
                turn_right_90(pmL, pmR);
            }
            else if(action_to_do == 4){ //reverse one square and then turn left 90
                reverse_after_read(pmL, pmR);
                reverse_one_square(pmL, pmR);
                turn_left_90(pmL, pmR);
            }
            else if(action_to_do == 5){ //turn left 135
                reverse_after_read(pmL, pmR);
                turn_left_135(pmL, pmR);
            }
            else if(action_to_do == 6){ //turn right 135
                reverse_after_read(pmL, pmR);
                turn_right_135(pmL, pmR);
            }
            // we've reached the white wall we need to turn around, and  leave this loop
            // then we will go to retrace loop
            else if(action_to_do == 7){
                reverse_after_read(pmL, pmR);
                turn_right_90(pmL, pmR);
                turn_right_90(pmL, pmR);
                break;
            }
        __delay_ms(5);
        }
    }

    
    //example history could be:
    //timings: [2,5,4,7,8,3,8]
    //actions: [R,L,R,R,R,L,END]
    int upcoming_action = actions_completed - 2;
    // we will iterate backwards through the array of actions done in our history
    // this while loop ends once the index is less than 0 (we have completed all actions)
    going_forward = false;
    
    //remove the 'reverse after read' equivalent value from timings
    for(char i = 0; i < 20; i +=1){ //iterate through the comparison array and find the smallest value
        timings[i] -= 2000; // calibrated value
        if(i > 7){ //remove more from the actions which reversed one square
              timings[i] -= 4000; // calibrated value
        }
    }
    

    // RETRACE LOOP
    while(upcoming_action >= 0){
        if(!going_forward){
            resetTimer0();
            fullSpeedAhead(pmL, pmR);
            going_forward = true;
        }
        //instead of measuring a colour, you meaure if you're above the length of time you went forwarrd when you were on this path
        measured_time = get16bitTMR0val();
        if(measured_time > timings[upcoming_action + 1]){
            stop(pmL, pmR);
            going_forward = false;
            action_to_do = invert_action(actions[upcoming_action]);
            // DO THIS ACTION
            if(action_to_do == 0){ //turn right 90
                turn_right_90(pmL, pmR);
            }
            else if(action_to_do == 1){ //turn left 90
                turn_left_90(pmL, pmR);
            }
            else if(action_to_do == 2){ //180 turn -> turn right twice (less calibration error)
                turn_right_90(pmL, pmR);
                turn_right_90(pmL, pmR);
            }
            else if(action_to_do == 5){ //reverse one square and then turn right 90
                turn_right_135(pmL, pmR);
            }
            else if(action_to_do == 6){ //reverse one square and then turn left 90
                turn_left_135(pmL, pmR);
            }
            else if(action_to_do == 8){
                turn_left_90;
            }
            else if(action_to_do == 9){
                turn_right_90;
            }
        upcoming_action -=1 ;
        }
    __delay_ms(10);
    }
    stop(pmL, pmR); //make sure buggy is stopped
}



