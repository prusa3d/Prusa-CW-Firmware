#include "Selftest.h"


bool callback = false;
bool helper = true;

void tCountDownComplete()
{
	callback = true;
}

CSelftest::CSelftest() : phase(0), cover_test(false), tank_test(false), vent_test(false), heater_test(false),
						 rotation_test(false), led_test(false), fan1_speed(10), fan2_speed(10), first_loop(true), measured_state(false),
						 prev_measured_state(false), counter(0)
{
	fan_tacho[0] = fan_tacho[1] = 0;
}

CSelftest::~CSelftest()
{
}

void CSelftest::universal_pin_test(){
	if(first_loop){
		prev_measured_state = measured_state;
	  	first_loop = false;
	}
	if(measured_state != prev_measured_state){
		prev_measured_state = measured_state;
	  	counter++;
	  	if(counter > 5 && phase == 1)
	  		cover_test = true;
	  	else if(counter > 5 && phase == 2)
	  		tank_test = true;
	}
}

void CSelftest::ventilation_test(bool f1_error, bool f2_error){
	if(first_loop == true){
		tCountDown.setCounter(0, 1, 0, tCountDown.COUNT_DOWN, tCountDownComplete);		//fans will do 1 minute
		tCountDown.start();
		first_loop = false;
	}
	if(callback == false){
		tCountDown.run();
		byte currSec = tCountDown.getCurrentSeconds();
		if(currSec % 10 == 0){
			if(fan1_speed + 20 <= 100 && helper){
				fan1_speed += 20;
				fan2_speed += 20;
				helper = false;
			}
		} else
			helper = true;

		if(f1_error || f2_error){
			tCountDown.stop();
			fan1_speed = fan2_speed = 10;
			measured_state = f1_error;			//variable recycling
			prev_measured_state = f2_error;
			vent_test = true;
		}
	 } else {
		 tCountDown.stop();
		 fan1_speed = fan2_speed = 10;
		 vent_test = true;
	 }
}

bool CSelftest::is_first_loop(){
	return first_loop;
}

const char * CSelftest::print(){
	switch (phase) {
	case 1:
		if(!cover_test){
			if(!measured_state)
				return "Open the cover";
			else
				return "Close the cover";
		} else
			return "Test Successful";
		break;
	case 2:
		if(!tank_test){
			if(!measured_state)
				return "Remove IPA tank";
		    else
		    	return "Insert IPA tank";
		} else
		    	return "Test Successful";
		break;
	case 3:
		if(!vent_test)
			return "Fan test";
		else {
			if (measured_state || prev_measured_state)
				return "Test Failed!";
			else
				return "Test Successful";
		}
	case 4:
		if(!led_test)
			return "LED test";
		else
			return "Test successful";
		break;
	case 5:
		if(!heater_test){
			if(fan1_speed == 0)
				return "Remove IPA tank";
			else
				return "Heater test";
		} else {
			if(measured_state)
				return "Test failed";
			else
				return "Test Successful";
		}
		break;

	case 6:
		if(!rotation_test)
			return "Rotation test";
		else
			return "Test Successful";
		break;

	default:
		return "";
		break;
	}
}


void CSelftest::clean_up(){
	first_loop = true;
	counter = 0;
	measured_state = prev_measured_state = false;
	fan1_speed = fan2_speed = 10;
	callback = false;
	helper = false;
}

void CSelftest::measure_state(bool tmp){
	measured_state = tmp;
}

void CSelftest::LED_test(){
	if(first_loop == true){
		tCountDown.setCounter(0, 10, 0, tCountDown.COUNT_DOWN, tCountDownComplete);		//leds will shine 10 minutes
		tCountDown.start();
		first_loop = false;
	}
	if(callback == false){
		tCountDown.run();
	} else {
		tCountDown.stop();
		led_test = true;
	}
}

bool CSelftest::motor_rotation_timer(){
	if(first_loop){
		tCountDown.setCounter(0, 0, 10, tCountDown.COUNT_DOWN, tCountDownComplete);			//10sec time periods
		tCountDown.start();
		helper = true;
		return true;
	}

	if(callback == false){
		tCountDown.run();
	} else {
		tCountDown.stop();
		callback = false;
		tCountDown.restart();
		return true;
	}
	return false;
}
void CSelftest::set_first_loop(const bool tmp){
	first_loop = tmp;
}

void CSelftest::heat_test(bool heater_error){
	if(first_loop == true){
			tCountDown.setCounter(0, 10, 0, tCountDown.COUNT_DOWN, tCountDownComplete);
			tCountDown.start();
			first_loop = false;
	}
	if(callback == false){
		tCountDown.run();

		if(heater_error){
			tCountDown.stop();
			measured_state = true;
			heater_test = true;
		}
	} else {
		tCountDown.stop();
		heater_test = true;
	}
}
