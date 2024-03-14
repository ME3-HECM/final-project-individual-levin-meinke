#include <xc.h>
#include "dc_motor.h"


// function initialise T2 and CCP for DC motor control
void initDCmotorsPWM(int PWMperiod){
    //initialise your TRIS and LAT registers for PWM  
    
    //configure PPS to map CCP modules to pins
    RE2PPS=0x05; //CCP1 on RE2
    RE4PPS=0x06; //CCP2 on RE4
    RC7PPS=0x07; //CCP3 on RC7
    RG6PPS=0x08; //CCP4 on RG6

    // timer 2 config
    T2CONbits.CKPS=0b100; // 1:16 prescaler
    T2HLTbits.MODE=0b00000; // Free Running Mode, software gate only
    T2CLKCONbits.CS=0b0001; // Fosc/4

    // Tpwm*(Fosc/4)/prescaler - 1 = PTPER
    // 0.0001s*16MHz/16 -1 = 99
    T2PR=99; //Period reg 10kHz base period
    T2CONbits.ON=1;
    
    //setup CCP modules to output PMW signals
    //initial duty cycles 
    CCPR1H=0; //0% duty cycle; CCPR1H=T2PR -> 100% duty cycle
    CCPR2H=0; 
    CCPR3H=0; 
    CCPR4H=0; 
    
    //use tmr2 for all CCP modules used
    CCPTMRS0bits.C1TSEL=0;
    CCPTMRS0bits.C2TSEL=0;
    CCPTMRS0bits.C3TSEL=0;
    CCPTMRS0bits.C4TSEL=0;
    
    //configure each CCP
    CCP1CONbits.FMT=1; // left aligned duty cycle (we can just use high byte)
    CCP1CONbits.CCP1MODE=0b1100; //PWM mode  
    CCP1CONbits.EN=1; //turn on
    
    CCP2CONbits.FMT=1; // left aligned
    CCP2CONbits.CCP2MODE=0b1100; //PWM mode  
    CCP2CONbits.EN=1; //turn on
    
    CCP3CONbits.FMT=1; // left aligned
    CCP3CONbits.CCP3MODE=0b1100; //PWM mode  
    CCP3CONbits.EN=1; //turn on
    
    CCP4CONbits.FMT=1; // left aligned
    CCP4CONbits.CCP4MODE=0b1100; //PWM mode  
    CCP4CONbits.EN=1; //turn on
    
    //map CPP modules output to pin
    RE2PPS=0x05; // CCP1 output is connected to pin RE2
    RE4PPS=0x06; // CCP2 "" RE4
    RC7PPS=0x07; // CCP3 "" RC7
    RG6PPS=0x08; // CCP4 "" RG6
    
    //set tris to output on pins
    TRISEbits.TRISE2=0; //RE2
    TRISEbits.TRISE4=0; //RE4
    TRISCbits.TRISC7=0; //RC7
    TRISGbits.TRISG6=0; //RG6
    
    //set lat registers on pins
    LATEbits.LATE2=0;
    LATCbits.LATC7=0;
    LATEbits.LATE4=0;
    LATGbits.LATG6=0;
}


// function to set CCP PWM output from the values in the motor structure
void setMotorPWM(struct DC_motor *m)
{
    unsigned char posDuty, negDuty; //duty cycle values for different sides of the motor
    
    if(m->brakemode) {
        posDuty=m->PWMperiod - ((unsigned int)(m->power)*(m->PWMperiod))/100; //inverted PWM duty
        negDuty=m->PWMperiod; //other side of motor is high all the time
    }
    else {
        posDuty=0; //other side of motor is low all the time
		negDuty=((unsigned int)(m->power)*(m->PWMperiod))/100; // PWM duty
    }
    
    if (m->direction) {
        *(m->posDutyHighByte)=posDuty;  //assign values to the CCP duty cycle registers
        *(m->negDutyHighByte)=negDuty;       
    } else {
        *(m->posDutyHighByte)=negDuty;  //do it the other way around to change direction
        *(m->negDutyHighByte)=posDuty;
    }
}

//function to make the robot go straight
void fullSpeedAhead(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 0; //left side forward
    mR->direction = 0; //right side forward
    
    while ((mL->power) < 100 || (mR->power) < 100){ //while motors are below 100 power
        if ((mL->power) < 100) (mL->power) += 1; //increment left
        if ((mR->power) < 100) (mR->power) += 1; //increment right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(2); //delay before repeating (gradual increase in speed)
    }
}

//function to stop the robot 
void stop(struct DC_motor *mL, struct DC_motor *mR){
    while ((mL->power) > 0 || (mR->power) > 0){ //while motors are running
        if ((mL->power) > 0) (mL->power) -= 1; //increment down left
        if ((mR->power) > 0) (mR->power) -= 1; //increment down right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_us(25); //delay before repeating (gradual decrease in speed)
    }
}

//function to make the robot turn left 90 deg
void turn_left_90(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 1; //left side backwards
    mR->direction = 0; //right side forward
            
    while ((mL->power) < 70 || (mR->power) < 70){ //while motors are below 70 power
        if ((mL->power) < 70) (mL->power) += 1; //increment down left
        if ((mR->power) < 70) (mR->power) += 1; //increment down right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(1); //delay before repeating (gradual decrease in speed)
    }    
    __delay_ms(440); //calibrate for how long it takes to turn
    stop(mL, mR);
}

//function to make the robot turn right 90 deg
void turn_right_90(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 0; //left side forward
    mR->direction = 1; //right side backwards
            
    while ((mL->power) < 70 || (mR->power) < 70){ //while motors are below 100 power
        if ((mL->power) < 70) (mL->power) += 1; //increment down left
        if ((mR->power) < 70) (mR->power) += 1; //increment down right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(1); //delay before repeating (gradual decrease in speed)
    }
    __delay_ms(440); //calibrate for how long it takes to turn
    stop(mL, mR);
}

//function to make the robot left right 135 deg
void turn_left_135(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 1; //left side backwards
    mR->direction = 0; //right side forward
            
    while ((mL->power) < 70 || (mR->power) < 70){ //while motors are below 70 power
        if ((mL->power) < 70) (mL->power) += 1; //increment down left
        if ((mR->power) < 70) (mR->power) += 1; //increment down right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(1); //delay before repeating (gradual decrease in speed)
    }    
    __delay_ms(700); //calibrate for how long it takes to turn
    stop(mL, mR);
}

//function to make the robot turn right 135 deg
void turn_right_135(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 0; //left side forward
    mR->direction = 1; //right side backwards
            
    while ((mL->power) < 70 || (mR->power) < 70){ //while motors are below 100 power
        if ((mL->power) < 70) (mL->power) += 1; //increment down left
        if ((mR->power) < 70) (mR->power) += 1; //increment down right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(1); //delay before repeating (gradual decrease in speed)
    }
    __delay_ms(700); //calibrate for how long it takes to turn
    stop(mL, mR);
}

void reverse_after_read(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 1; //left side back
    mR->direction = 1; //right side back
    
    while ((mL->power) < 70 || (mR->power) < 70){ //while motors are below 100 power
        if ((mL->power) < 70) (mL->power) += 1; //increment left
        if ((mR->power) < 70) (mR->power) += 1; //increment right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(1); //delay before repeating (gradual increase in speed)
    }
    __delay_ms(225); //calibrate for how long it takes to go back
    stop(mL, mR);
    __delay_ms(250); //let buggy settle before proceeding
}

void reverse_one_square(struct DC_motor *mL, struct DC_motor *mR){
    mL->direction = 0; //left side forward
    mR->direction = 0; //right side forward
    
    while ((mL->power) < 70 || (mR->power) < 70){ //while motors are below 100 power
        if ((mL->power) < 70) (mL->power) += 1; //increment left
        if ((mR->power) < 70) (mR->power) += 1; //increment right
        //call set motorPWM functions
        setMotorPWM(mL);
        setMotorPWM(mR);
        __delay_ms(1); //delay before repeating (gradual increase in speed)
    }
    __delay_ms(500); //calibrate for how long it takes to go back
    stop(mL, mR);
    __delay_ms(250); //let the buggy settle
}





