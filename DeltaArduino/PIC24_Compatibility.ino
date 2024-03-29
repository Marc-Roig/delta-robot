//All functions to make code compatible both with arduino and PIC24FJ
#include "Config.h"

Servo servos[4];

void Serial_begin(int baudrate) {

	Serial.begin(9600);

}

void Serial_write(char* data) {

	if (data == " ") Serial.write(' '); //there seems to be a problem printing " "
	Serial.write(data);

}

void Serial_print(int num) {

	Serial.print(num);

}

void Serial_println(int num) {

	Serial.println(num);

}

char Serial_read() {

	return Serial.read();

}

int Serial_available() {

	return Serial.available();

}

void servo_writeMicroseconds(int duty_cycle, int servo_num) {

	servos[servo_num].writeMicroseconds(duty_cycle);

}

void servo_write(int angle, int servo_num) {

	servos[servo_num].write(angle);

}

void servo_attach(int pin, int servo_num) {

	servos[servo_num].attach(pin);

}