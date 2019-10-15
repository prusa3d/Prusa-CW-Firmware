#include "Selftest.h"

CSelftest::CSelftest() : phase(0), cover_test(false), tank_test(false), vent_test(false), heater_test(false),
						 rotation_test(false), led_test(false), fan1_speed(10), fan2_speed(10), cover_down(false),
						 isCounterRunning(false), fail_flag(false), measured_state(false), helper(true), first_loop(true),
						 prev_measured_state(false), counter(0)
{
	fan_tacho[0] = fan_tacho[1] = 0;
	callback = false;
}

CSelftest::~CSelftest()
{
}

void CSelftest::tCountDownComplete()
{
	callback = true;
}

bool CSelftest::universal_pin_test(){
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
	  	return true;
	}
	return false;
}

void CSelftest::ventilation_test(bool f1_error, bool f2_error){
	if(first_loop == true){
		tCountDown.setCounter(0, 1, 0, tCountDown.COUNT_DOWN, tCountDownComplete);		//fans will do 1 minute
		tCountDown.start();
		first_loop = false;
	}
	if(callback == false){
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
			fail_flag = true;
			vent_test = true;
		}
	 } else {
		 fan1_speed = fan2_speed = 10;
		 vent_test = true;
	 }
}

bool CSelftest::is_first_loop(){
	return first_loop;
}

const char * CSelftest::print(){
	switch (phase) {
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
		if(!led_test){
			if(cover_down)
				return "LED test";
			else
				return "Close the cover";
		} else {
			if(fail_flag == true)
				return "Test Failed";
			else
				return "Test successful";
		}
		break;
	case 5:
		if(!heater_test){
			if(fan1_speed == 0)
				return "Remove IPA tank";
			else {
				if(fan2_speed == 0)
					return "Close the cover";
				else
					return "Heater test";
			}
		} else {
			if(fail_flag == true)
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
	isCounterRunning = false;
	fail_flag = false;
}

void CSelftest::LED_test(){
	if(first_loop == true){
		tCountDown.setCounter(0, 10, 0, tCountDown.COUNT_DOWN, tCountDownComplete);		//leds will light 10 minutes
		tCountDown.start();
		isCounterRunning = true;
		first_loop = false;
	}
	if(callback == false){
		if(tCountDown.isStopped()){
			tCountDown.start();
			isCounterRunning = true;
		}
	} else {
		isCounterRunning = false;
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

	if(callback == true){
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
			isCounterRunning = true;
	}
	if(callback == false){
		if(heater_error){
			tCountDown.stop();
			fail_flag = true;
			heater_test = true;
			isCounterRunning = false;
		}
	} else {
		heater_test = true;
		isCounterRunning = false;
	}
}
