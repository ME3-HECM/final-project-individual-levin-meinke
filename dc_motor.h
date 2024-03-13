#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H

#include <xc.h>

#define _XTAL_FREQ 64000000

struct DC_motor { //definition of DC_motor structure
    char power;         //motor power, out of 100
    char direction;     //motor direction, forward(1), reverse(0)
    char brakemode;		// short or fast decay (brake or coast)
    unsigned int PWMperiod; //base period of PWM cycle
    unsigned char *posDutyHighByte; //PWM duty address for motor +ve side
    unsigned char *negDutyHighByte; //PWM duty address for motor -ve side
};


//functions used during the maze
void initDCmotorsPWM(int PWMperiod); // function to setup PWM
void setMotorPWM(struct DC_motor *m); //set power of motors
void fullSpeedAhead(struct DC_motor *mL, struct DC_motor *mR); //go straight
void stop(struct DC_motor *mL, struct DC_motor *mR); //stop
void turn_left_90(struct DC_motor *mL, struct DC_motor *mR); //left 90 deg
void turn_right_90(struct DC_motor *mL, struct DC_motor *mR); //right 90 deg
void turn_left_135(struct DC_motor *mL, struct DC_motor *mR); //left 135 deg
void turn_right_135(struct DC_motor *mL, struct DC_motor *mR); //right 135 deg
void reverse_after_read(struct DC_motor *mL, struct DC_motor *mR); //reverse after reading color card
void reverse_one_square(struct DC_motor *mL, struct DC_motor *mR); //reverse one square

#endif
