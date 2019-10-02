#ifndef SPEED_CONTROL_H
#define SPEED_CONTROL_H

#define SERIAL_COM_DEBUG

#include "Arduino.h"

struct speed_control_t {

	const unsigned int rotation_start = 200;	//smaller = faster
	const unsigned int min_curing_speed = 220;	//smaller = faster
	const unsigned int max_curing_speed = 25;	//smaller = faster
	const unsigned int min_washing_speed = 70;	//smaller = faster
	const unsigned int max_washing_speed = 16;	//smaller = faster
	unsigned int uni_speed_var;
	bool speed_up;
	unsigned int set_curing_speed;
	unsigned int set_washing_speed;
	byte washing_speed;
	byte curing_speed;

	speed_control_t() : uni_speed_var(200), speed_up(false), set_curing_speed(220), set_washing_speed(200), washing_speed(10), curing_speed(1) {}

	void speed_configuration(bool curing_mode){
		if (curing_mode == true) {
		    set_curing_speed = map(curing_speed, 1, 10, min_curing_speed, max_curing_speed);
		    uni_speed_var = set_curing_speed;
		} else {
		    set_washing_speed = map(washing_speed, 1, 10, min_washing_speed, max_washing_speed);
		    uni_speed_var = rotation_start;
		    speed_up = true;
		}
	}

	void speeding_up(){
		if (uni_speed_var > set_washing_speed) {
		    	  if(uni_speed_var > min_washing_speed + 5)
		    		  uni_speed_var -= 4;
		    	  uni_speed_var--;
		#ifdef SERIAL_COM_DEBUG
		        SerialUSB.print(uni_speed_var);
		        SerialUSB.write('.');
		        SerialUSB.print(set_washing_speed);
		        SerialUSB.write('\n');
		#endif
		} else {
			speed_up = false;
		}
	}

};

#endif  //SPEED_CONTROL_H
