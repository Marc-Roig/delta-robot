#include "Config.h"

//------------------------//

void move_servos(bool move_servo1, bool move_servo2, bool move_servo3, bool move_servo4) {

    bool move_servos[] = {move_servo1, move_servo2, move_servo3, move_servo4};

    //TODO: If servo arrived at destination (need to read potentiometer)
    //      turn off servo. Turn on again when dc changes
    
    for (int i = 0; i < 4; i ++) {

        if (move_servos[i]) {

            if (servoinfo[i].move_servo_from == MOVE_SERVO_FROM_ANGLE) {

                update_dc_from_angle(i);  //Update dc

            }
            else if (servoinfo[i].move_servo_from == MOVE_SERVO_FROM_DC) {

                // update_angle_from_dc(i);  //Update angles

            }
            else if (servoinfo[i].move_servo_from == MOVE_SERVO_FROM_XYZ) { 

                // if (i == 0) update_angles_from_xyz(); //All servo angles are updated at once
                update_dc_from_angle(i);

            }

            servo_writeMicroseconds(servoinfo[i].duty_cycle + servoinfo[i].dc_offset, i);

        }

    }

    //To avoid doing the calculations 3 times each loop.
    //Angles are already updated if the mode was duty_cycle
    //Arms servos need to have the same "move_servo_from" so
    //if the first isnt XYZ the others will not be either.

    // if (servoinfo[0].move_servo_from != "XYZ") update_xyz_from_angles();

}

void update_angle_from_dc(int servo_num) {

    servoinfo[servo_num].angle = (float)(servoinfo[servo_num].duty_cycle - servoinfo[servo_num].n) / servoinfo[servo_num].m;

}

void update_dc_from_angle(int servo_num) {

    int new_duty_cycle = (int) (servoinfo[servo_num].angle * servoinfo[servo_num].m + servoinfo[servo_num].n);

    check_servo_change_direction(servo_num, new_duty_cycle);
    servoinfo[servo_num].duty_cycle = new_duty_cycle;

} 


/*********************************************************************
* Function: check_servo_change_direction(int num_servo, int new_duty_cycle);
*
* Overview: Servos need an extra duty cycle value to compensate the slack
*           when changing direction. 
*
* PreCondition: The new duty cycle value has to be checked in this function
*               first before changing the duty_cycle value of servoinfo.
*
* Input: bool - Will move the servo 1
*        bool - Will move the servo 2
*        bool - Will move the servo 3
*        bool - Will move the servo 4
*
* Output: none
*
********************************************************************/

void check_servo_change_direction(unsigned int num_servo, unsigned int new_duty_cycle) {

  int min_step_to_change_dir = 20;

  if (new_duty_cycle > servoinfo[num_servo].duty_cycle + min_step_to_change_dir) {

    if (servoinfo[num_servo].last_direction == COUNTERCLOCKWISE) {

        // Serial.write("changed to CLOCKWISE\n");

        servoinfo[num_servo].last_direction = CLOCKWISE;
        servoinfo[num_servo].dc_offset += servoinfo[num_servo].slack_compensation_val;

    }

  }
  else if (new_duty_cycle < servoinfo[num_servo].duty_cycle - min_step_to_change_dir) {

    if (servoinfo[num_servo].last_direction == CLOCKWISE) {

        // Serial.write("changed to COUNTERCLOCKWISE\n");

        servoinfo[num_servo].last_direction = COUNTERCLOCKWISE;
        servoinfo[num_servo].dc_offset -= servoinfo[num_servo].slack_compensation_val;

    }

  }

}


/********************************************************************
* Function: init_ServoInfo(struct ServoInfo* servo_inf, int max_duty_cycle_, int min_duty_cycle_, int slack_compensation_val_, float m_, float n_);
*
* Overview: none
*
* PreCondition: none
*
* Input: ServoInfo*                     - the servo information struct
*        int max_duty_cycle             - max duty cycle of the servo 
*        int min_duty_cycle             - mix duty cycle of the servo 
*        int slack_compensation_val     - value of compensation when servo is changing direction 
*        int m                          - slope of the equation duty_cycle - angle 
*        int n                          - offset of the equation duty_cycle - angle
*
* Output: none
*
*******************************************************************/

void init_ServoInfo(ServoInfo* servo_inf, int max_duty_cycle_, int min_duty_cycle_, int slack_compensation_val_, float m_, float n_) {

    servo_inf->angle = 90.0;
    
    servo_inf->max_duty_cycle = max_duty_cycle_;
    servo_inf->min_duty_cycle = min_duty_cycle_;

    servo_inf->dc_offset = 0;

    servo_inf->mean_dc = (max_duty_cycle_ + min_duty_cycle_)/2; 
    servo_inf->duty_cycle = servo_inf->mean_dc - 500;

    servo_inf->last_direction = CLOCKWISE;
    servo_inf->slack_compensation_val = slack_compensation_val_;

    servo_inf->m = m_;
    servo_inf->n = n_;
    
}

/*********************************************************************
* Function: void servos_initial_positions(bool move_servo1, bool move_servo2, bool move_servo3);
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

void servos_initial_positions(bool move_servo1, bool move_servo2, bool move_servo3, bool move_servo4) {

    bool move_servos[] = {move_servo1, move_servo2, move_servo3, move_servo4};

    for (int i = 0; i < 3; i++) {
        
        if (move_servos[i]) {
            servo_writeMicroseconds((55 * servoinfo[i].m + servoinfo[i].n), i);
        }
    }

    delay(1000);

    for (int i = 0; i < 3; i++) {
        
        if (move_servos[i]) {

            servoinfo[i].duty_cycle = 90 * servoinfo[i].m + servoinfo[i].n - servoinfo[i].slack_compensation_val;
            servo_writeMicroseconds(servoinfo[i].duty_cycle, i); //left servo moved counterclowised

        }
    }

}

/*********************************************************************
* Function: init_servos();
*
* Overview: All variables related to the servos are initialized
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
********************************************************************/

void init_servos(bool move_servo1, bool move_servo2, bool move_servo3, bool move_servo4) {
  
    if (move_servo1) {
        servos[0].attach(SERVO1_PIN);
        init_ServoInfo(&servoinfo[0], MAX_DC_SERVO1, MIN_DC_SERVO1, SERVO1_COMPENSATION_VAL, SERVO1_M_ANGLE_TO_DC, SERVO1_N_ANGLE_TO_DC);
    }

    if (move_servo2) {
        servos[1].attach(SERVO2_PIN);
        init_ServoInfo(&servoinfo[1], MAX_DC_SERVO2, MIN_DC_SERVO2, SERVO2_COMPENSATION_VAL, SERVO2_M_ANGLE_TO_DC, SERVO2_N_ANGLE_TO_DC);
    }

    if (move_servo3) {
        servos[2].attach(SERVO3_PIN);
        init_ServoInfo(&servoinfo[2], MAX_DC_SERVO3, MIN_DC_SERVO3, SERVO3_COMPENSATION_VAL, SERVO3_M_ANGLE_TO_DC, SERVO3_N_ANGLE_TO_DC);
    }

    if (move_servo4) {
        servos[3].attach(SERVO4_PIN);
        init_ServoInfo(&servoinfo[3], MAX_DC_SERVO4, MIN_DC_SERVO4, SERVO4_COMPENSATION_VAL, SERVO3_M_ANGLE_TO_DC, SERVO3_N_ANGLE_TO_DC);
    }

    servoinfo[0].move_servo_from = MOVE_SERVO_1_FROM;
    servoinfo[1].move_servo_from = MOVE_SERVO_2_FROM;
    servoinfo[2].move_servo_from = MOVE_SERVO_3_FROM;
    servoinfo[3].move_servo_from = MOVE_SERVO_4_FROM;

    servos_initial_positions(move_servo1, move_servo2, move_servo3, move_servo4); //Move every servo to the initial position

}

void set_servo_movement_with_dc(bool set_servo1, bool set_servo2, bool set_servo3, bool set_servo4) {

    if (set_servo1) servoinfo[0].move_servo_from = MOVE_SERVO_FROM_DC;
    if (set_servo2) servoinfo[1].move_servo_from = MOVE_SERVO_FROM_DC;
    if (set_servo3) servoinfo[2].move_servo_from = MOVE_SERVO_FROM_DC;
    if (set_servo4) servoinfo[3].move_servo_from = MOVE_SERVO_FROM_DC;
    
}
