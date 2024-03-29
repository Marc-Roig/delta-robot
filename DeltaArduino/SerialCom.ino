#include "Config.h"

CommandsBuffer buffer;
bool command_recieved = false;
int serial_mode = ASK_FOR_ANGLES;

//Comands 

// G03 XXX XXX XXX - Angles of servos (S1, S2, S3, S4 GRIP)
// G04 XXX - % Speed Overall
// G05 XYZ - Position end of arm
// G06 - End Of Stream
// G09 - Emergency Stop

// G20 - Send me Angles

void serial_com_with_simulator() { //FUNCITON TO CALL IN MAIN

    static unsigned long startTimeSim = millis();

    if(command_recieved && (millis() - startTimeSim) > SERIAL_DELAY_MS) {

        do { parse_command(buffer.command[buffer.start]);
        } while (!inc_buffer_start_pointer());

        //When command End of Stream is read I send all
        //the information and wait for more
        command_recieved = false; //Buffer is empty
        startTimeSim = millis();
    }

    else {
        check_serial();
    }

}

//--COMMAND--//

void parse_command(char command[SERIAL_COMMAND_MAX_LEN]) {

    int command_num = chars_to_int('0', command[1], command[2]);
    
    switch(command_num) {

        case END_OF_STREAM:             serial_next_instruction();
                                        break;
            
        case MOVE_SERVOS:               serial_recieve_angles(command);
                                        break;
            
        case MOVE_EF:                   serial_recieve_ef_pos(command);
                                        break;

        case SEND_ME_ANGLES:            serial_send_angles();
                                        break;

        case SEND_ME_DC:                serial_send_dc();
                                        break;

        case CHANGE_TO_CALIBRATION:     deltainfo.mode = CALIBRATION_MODE;
                                        calibration_start(true, true, true, false);
                                        Serial_write("Changing to Calibration Mode\n");
                                        break;

        case CHANGE_TO_JOYSTICK:        deltainfo.mode = JOYSTICK_MODE;
                                        Serial_write("Changing to Joystick Mode\n");
                                        init_joystick();
                                        break;

        case CHANGE_TO_SEQUENCE:        deltainfo.mode = SEQUENCE_MODE;
                                        Serial_write("Changing to Sequence Mode\n");
                                        init_joystick();
                                        init_sequence();
                                        break;

        case MOVE_SERVO_TO_HOME:        Serial_write("Moving servos to home");
                                        servos_power_off_positions(true, true, true, true);
                                        break;
 
        default:                        Serial_write("SERIAL BAD REQUEST\n");
                                        break;

    }
  
}

void serial_next_instruction() {

  switch (serial_mode) {

    case ASK_FOR_ANGLES:  //serial_send_angles();
                          send_command_header(END_OF_STREAM, true);
                          break;

    default:              break;
    
  }

}

//--RECIEVE DATA--//
void check_serial() {

  static int i = 0; //Number of char recieved from current command
  char incomingByte;
  
  if (buffer.full) {
    //How to get the last one.
    //Get the first one, introduce the G06
    //Wait some time to start again the communication
  }
  else if (Serial_available() > 0 ) {

    incomingByte = Serial_read();

    if (incomingByte == '\0') return; 

    buffer.command[buffer.end_][i] = incomingByte;
    i++;

    if (incomingByte == '\n') {
      buffer.command_len[buffer.end_] = i; 

      i = 0;

      if (buffer.command[buffer.end_][2] == (END_OF_STREAM + '0')) {
        command_recieved = true;
      }

      inc_buffer_end_pointer();

    }
  }

}

bool inc_buffer_end_pointer() {

  buffer.end_ = (buffer.end_ + 1) % MAX_COMMANDS_NUM;
  buffer.empty = false;
  if (buffer.end_ == buffer.start) {
    buffer.full = true;
    return true;   
  }
  return false;

}

bool inc_buffer_start_pointer() {

  buffer.start = (buffer.start + 1) % MAX_COMMANDS_NUM;
  buffer.full = false;

  if (buffer.end_ == buffer.start) {
    buffer.empty = true;
    return true;
  }
  return false;

}

void init_buffer() {

  buffer.start = 0;
  buffer.end_ = 0;
  buffer.empty = true;
  buffer.full = false;
  
}

void serial_recieve_angles(char command[SERIAL_COMMAND_MAX_LEN]) {

  bool bad_request = false;
  
  for (int i = 4; i < 7; i++) {
    if (!is_alphanumeric(command[i])) bad_request = true; 
  }
  for (int i = 8; i < 11; i++) {
    if (!is_alphanumeric(command[i])) bad_request = true;
  }
  for (int i = 12; i < 15; i++) {
    if (!is_alphanumeric(command[i])) bad_request = true;
  }

  if (!bad_request) {
  //    Serial_write("GOOD REQUEST\n");
    servoinfo[0].angle = chars_to_int(command[4], command[5], command[6]);
    servoinfo[1].angle  = chars_to_int(command[8], command[9], command[10]);
    servoinfo[2].angle  = chars_to_int(command[12], command[13], command[14]);
  }
  else Serial_write("BAD REQUEST\n");

}

void serial_recieve_ef_pos(char command[SERIAL_COMMAND_MAX_LEN]) {

  bool bad_request = false;

  for(int i = 4; i < 7; i++) {
    if (!is_alphanumeric(command[i])) bad_request = true;
  }

  if (!bad_request) {

    #ifdef KINEMATICS_H
    // deltainfo.x = ...;
    // deltainfo.y = ...;
    // deltainfo.x = ...;
    #endif

  }

}

//--SEND DATA--//
void serial_send_angles() {

  send_command_header(MOVE_SERVOS, false);
  for (int i = 0; i < 3; i++) {
    Serial_write(int_to_char_3digits(servoinfo[i].angle));
    
    if (i == 2) Serial_write("\n");
    else Serial_write(" "); 
  }

}

void serial_send_dc() {

  send_command_header(NEW_DC_VALUES, false);
  for (int i = 0; i < 3; i++) {
    Serial_write(int_to_char_4digits(servoinfo[i].duty_cycle));
    
    if (i == 2) Serial_write("\n");
    else Serial_write(" "); 
  }

}

void serial_send_speed() {

  send_command_header(CHANGE_SPEED, false);
  // Serial_write(end_effector_speed + '0');
  Serial_write("\n");

}

void serial_send_ef_pos() {

  send_command_header(MOVE_EF, false);
  // Serial_write(end_effector_pos[0]+'0');
  // Serial_write(end_effector_pos[1]+'0');
  // Serial_write(end_effector_pos[1]+'0');
  Serial_write("\n");

}

void send_command_header(int command_num, bool end_with_new_line) {

  Serial_write("G");
  Serial_write(int_to_char_2digits(command_num));
  if (end_with_new_line) Serial_write("\n");

  else Serial_write(" ");

}

void serial_write_dc_every_ms(int wait_time) {

    static unsigned long startMilis = millis();
    
    if ((millis() - startMilis) > wait_time) {

        startMilis = millis();

        Serial_write("Duty cycle: ");
        Serial_print((int)servoinfo[0].duty_cycle);
        Serial_write(" - ");
        Serial_print((int)servoinfo[1].duty_cycle);
        Serial_write(" - ");
        Serial_println(servoinfo[2].duty_cycle);

    }

}


void serial_write_angles() {

    Serial_write("Angles: ");
    Serial_print((int)servoinfo[0].angle);
    Serial_write(" - ");
    Serial_print((int)servoinfo[1].angle);
    Serial_write(" - ");
    Serial_println(servoinfo[2].angle);

}

