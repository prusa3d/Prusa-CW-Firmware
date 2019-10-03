#include "SpeedControl.h"

CSpeedControl::CSpeedControl() : uni_speed_var(200), washing_speed(10), curing_speed(1),
								 speed_up(false), set_curing_speed(220), set_washing_speed(200) {}

CSpeedControl::~CSpeedControl() {}

void CSpeedControl::speed_configuration(bool curing_mode){
	if (curing_mode == true) {
	    set_curing_speed = map(curing_speed, 1, 10, min_curing_speed, max_curing_speed);
	    uni_speed_var = set_curing_speed;
	} else {
	    set_washing_speed = map(washing_speed, 1, 10, min_washing_speed, max_washing_speed);
	    uni_speed_var = rotation_start;
	    speed_up = true;
	}
}

void CSpeedControl::speeding_up(){
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
