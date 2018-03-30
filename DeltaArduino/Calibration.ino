#include "Config.h"

/*********************************************************************
* Function: void servo_calibration(bool move_servo1, bool move_servo2, bool move_servo3);
*
* Overview: Reads the buttons and potentiometer
*           and then moves the servos
*
* PreCondition: Needs to be called in the main loop
*
* Input: bool - Will move the servo 1
*        bool - Will move the servo 2
*        bool - Will move the servo 3
*
* Output: none
*
********************************************************************/

void servo_calibration(bool move_servo1, bool move_servo2, bool move_servo3, bool move_servo4) {

    static int change_dc_mode = CHANGE_WITH_BUTTONS;

    switch (change_dc_mode) {

        case CHANGE_WITH_POTENTIOMETER:     if (move_servo1) calibration_change_dc_potentiometer(0);
                                            if (move_servo2) calibration_change_dc_potentiometer(1);
                                            if (move_servo3) calibration_change_dc_potentiometer(2);
                                            if (move_servo4) calibration_change_dc_potentiometer(3);
                                            break;

        case CHANGE_WITH_BUTTONS:           if (move_servo1) calibration_change_dc_buttons(0);
                                            if (move_servo2) calibration_change_dc_buttons(1);
                                            if (move_servo3) calibration_change_dc_buttons(2);
                                            if (move_servo4) calibration_change_dc_buttons(3);
                                            break;

        case CHANGE_WITH_SERIAL:            calibration_change_dc_serial();
                                            break;

        default:                            break;

    } 
    
    calibration_change_dc_mode(&change_dc_mode);

    move_servos_from_dc(move_servo1, move_servo2, move_servo3, move_servo4);

    serial_write_dc_every_ms(1000);

}


/*********************************************************************
* Function: void calibration_start(bool move_servo1, bool move_servo2, bool move_servo3);
*
* Overview: Prepare board to calibrate servos
*
* PreCondition: Needs to be called in the setup function
*
* Input: bool - Will move the servo 1
*        bool - Will move the servo 2
*        bool - Will move the servo 3
*
* Output: none
*
********************************************************************/

void calibration_start(bool move_servo1, bool move_servo2, bool move_servo3, bool move_servo4) {

    pinMode(CHANGE_MODE_BUTTON_PIN, INPUT);
    pinMode(INCREASE_DC_BUTTON_PIN, INPUT);
    pinMode(DECREASE_DC_BUTTON_PIN, INPUT);
    pinMode(CHANGE_STEP_CHANGE_PIN, INPUT);

    calibration_initial_positions(move_servo1, move_servo2, move_servo3, move_servo4);

}


/*********************************************************************
* Function: void calibration_initial_positions(bool move_servo1, bool move_servo2, bool move_servo3);
*
* Overview: To reduce the slack of the gears all servos are moved
*           clockwise. To ensure they are left moved clockwise
*           they are first moved counterclockwise just in case they
*           coudln't move any more clockwise.
*
* PreCondition: none
*
* Input: bool - Will move the servo 1
*        bool - Will move the servo 2
*        bool - Will move the servo 3
*
* Output: none
*
********************************************************************/

void calibration_initial_positions(bool move_servo1, bool move_servo2, bool move_servo3, bool move_servo4) {

    bool move_servos[] = {move_servo1, move_servo2, move_servo3, move_servo4};

    for (int i = 0; i < 3; i++) {
        
        if (move_servos[i]) {
            servos[i].writeMicroseconds((45 * servoinfo[i].m + servoinfo[i].n));
        }
    }

    delay(1000);

    for (int i = 0; i < 3; i++) {
        
        if (move_servos[i]) {
            servoinfo[i].duty_cycle = 90 * servoinfo[i].m + servoinfo[i].n - servoinfo[i].slack_compensation_val;
            servos[i].writeMicroseconds(servoinfo[i].duty_cycle); //left servo moved counterclowised
        }
    }

}


/*********************************************************************
* Function: void calibration_change_dc_mode(int* change_dc_mode);
*
* Overview: When the change mode button is pressed the duty
*           cycle changes from the potentiometer to the
*           buttons or visce versa
*
* PreCondition: All servoinfo values have to be set
*
* Input: int* - actual duty cycle change mode
*
* Output: none
*
********************************************************************/

void calibration_change_dc_mode(int* change_dc_mode) {

    static int S4 = 0;
    bool change_dc_m_button = digitalRead(CHANGE_MODE_BUTTON_PIN);
    
    if (change_dc_m_button && !S4) {
    
      *change_dc_mode = (*change_dc_mode + 1) % 3;
      S4 = 1;
      
    }
    else if (!change_dc_m_button) S4 = 0;

}


/*********************************************************************
* Function: void calibration_change_dc_potentiometer(int servo_num);
*
* Overview: Change duty cycle moving the potentiometer
*
* PreCondition: All servoinfo values have to be set
*
* Input: int - number of servo to move
*
* Output: none
*
********************************************************************/

void calibration_change_dc_potentiometer(int servo_num) {

    int min_dc = servoinfo[servo_num].min_duty_cycle;
    int max_dc = servoinfo[servo_num].max_duty_cycle;
    int last_direction = servoinfo[servo_num].last_direction;
    int compensation_val = servoinfo[servo_num].slack_compensation_val;

    int duty_cycle = map(analogRead(CALIBRATION_POTE_PIN) ,0 , 1023, min_dc, max_dc);

    check_servo_change_direction(servo_num, duty_cycle);

    servoinfo[servo_num].duty_cycle = duty_cycle;
}


/*********************************************************************
* Function: void calibration_change_dc_buttons(int servo_num);
*
* Overview: Change duty cycle pushing buttons
*
* PreCondition: All servoinfo values have to be set
*
* Input: int - number of servo to move
*
* Output: none
*
********************************************************************/

void calibration_change_dc_buttons(int servo_num) {

    static int S1 = 0, S2 = 0, S3 = 0;
    static int mode = 0; 

    int step1 = 20, step2 = 40, step3 = 100;
    int duty_cycle = servoinfo[servo_num].duty_cycle;

    bool chante_step_button = digitalRead(CHANGE_STEP_CHANGE_PIN);
    bool increase_dc_button = digitalRead(INCREASE_DC_BUTTON_PIN);
    bool decrease_dc_button = digitalRead(DECREASE_DC_BUTTON_PIN);
 
    //--INCREMENT duty_cycle--//
    if (increase_dc_button && !S1) {
        //Increase value depending on the mode
        switch(mode) {
            case 0:
                duty_cycle = duty_cycle + step1;
                break;
            case 1:
                duty_cycle = duty_cycle + step2;
                break;
            default:
            duty_cycle = duty_cycle + step3;
            break;
        }

        //Limit max angle
        if (duty_cycle >= servoinfo[servo_num].max_duty_cycle) duty_cycle = servoinfo[servo_num].max_duty_cycle;

        //Flank detection
        S1 = 1;

        //Change direction compensation
        if (servoinfo[servo_num].last_direction == CLOCKWISE) {
            servoinfo[servo_num].last_direction = COUNTERCLOCKWISE;
            duty_cycle += servoinfo[servo_num].slack_compensation_val;
        }

    }
    else if (!increase_dc_button) S1 = 0;

    //--DECREMENT duty_cycle--//
    if (decrease_dc_button && !S2) {
     
     switch(mode) {
        case 0:
            duty_cycle = duty_cycle - step1;
            break;
        case 1:
            duty_cycle = duty_cycle - step2;
            break;
        default:
            duty_cycle = duty_cycle - step3;
            break;
     }

     if (duty_cycle <= servoinfo[servo_num].min_duty_cycle) duty_cycle = servoinfo[servo_num].min_duty_cycle;

     S2 = 1;

     if (servoinfo[servo_num].last_direction == COUNTERCLOCKWISE) {
        servoinfo[servo_num].last_direction = CLOCKWISE;
        duty_cycle -= servoinfo[servo_num].slack_compensation_val;
     }

    }
    else if (!decrease_dc_button) S2 = 0;

    //--CHANGE STEP--//
    if (chante_step_button && !S3) {

        mode = (mode + 1) % 3; 
        S3 = 1;

    }
    else if (!chante_step_button) S3 = 0;

    servoinfo[servo_num].min_duty_cycle = duty_cycle;   //Store the value in the global variable

}


/*********************************************************************
* Function: void calibration_change_dc_serial();
*
* Overview: Change duty cycle typing angle in serial monitor
*           Only works with the three arms servos not with the
*           gripper.
*
* PreCondition: Always type three digits -> 034
*
* Input: none
*
* Output: none
*
********************************************************************/

void calibration_change_dc_serial() {

    static char input[] = {0, 0, 0};
    static int i = 0;

    if (Serial.available() > 0) {

        char incomingByte = Serial.read();
        if (incomingByte == '\n') {

            int number = chars_to_int(input[0], input[1], input[2]);

            if (number < 160 && number > 10) {

                servoinfo[0].angle = number;
                servoinfo[1].angle = number;
                servoinfo[2].angle = number;

            }

            input[0] = 0;
            input[1] = 0;
            input[2] = 0;
            
            i = 0;
        }

        if (i < 3 && is_alphanumeric(incomingByte)) {

            input[i] = incomingByte;
            i++;

        }

    }

}