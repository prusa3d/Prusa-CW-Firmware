#include "Selftest.h"


bool callback = false;
bool helper = true;

void tCountDownComplete()
{
	callback = true;
}

CSelftest::CSelftest() : phase(3), cover_test(false), tank_test(false), vent_test(false), heater_test(false),
						 rotation_test(false), led_test(false), fan1_speed(10), fan2_speed(10), first_loop(true), measured_state(false),
						 prev_measured_state(false), counter(0)
{
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
			if(fan1_speed < 100 && helper){
				fan1_speed += 20;
				fan2_speed += 20;
				helper = false;			//TODO: measuring speed every ten sec
			}
		} else
			helper = true;

		if(f1_error || f2_error){
			tCountDown.stop();
			fan1_speed = fan2_speed = 10;
			measured_state = f1_error;			//variable recycling
			prev_measured_state = f2_error;
			vent_test = false;
		}
			//fan_rpm() -> fan1_error ? fan2_error ?
			//fan_heater_rpm() -> fan3_error ?
	 } else {
		 tCountDown.stop();
		 fan1_speed = fan2_speed = 10;		//!!! Obcas se nedostane sem nebo nevim
		 if(measured_state == false && prev_measured_state == false)
			 vent_test = true;
	 }
}

bool CSelftest::isFirstLoop(){
	return first_loop;
}

const char * CSelftest::print(){
	switch (phase) {
	case 1:
		if(counter < 5){
			if(!measured_state)
				return "Open the cover";
			else
				return "Close the cover";
		} else {
			cover_test = true;
			return "Test Successful";
		}
		break;
	case 2:
		if(counter < 5){
			if(!measured_state)
				return "Remove IPA tank";
		    else
		    	return "Insert IPA tank";
		    } else {
		    	tank_test = true;
		    	return "Test Successful";
		    }
		break;
	case 3:
		if(!vent_test)
			return "Fan test [1 min]";
		else{
			if (measured_state || prev_measured_state)
				return "Test Failed!";
			else
				return "Test Successful";
		}
	case 4:
		if(!led_test){
			return "LED test [1 min]";
		} else
			return "Test successful";
		break;

	default:
		return "";
		break;
	}
}

void CSelftest::cleanUp(){
	first_loop = true;
	counter = 0;
	measured_state = prev_measured_state = false;
	fan1_speed = fan2_speed = 10;
	callback = false;
}

void CSelftest::measureState(bool tmp){
	measured_state = tmp;
}

void CSelftest::LED_test(){
	if(first_loop == true){
		tCountDown.restart();		//1 min
		first_loop = false;
	}
	if(callback == false){
		tCountDown.run();
	} else {
		tCountDown.stop();
		led_test = true;
	}
}

void CSelftest::motor_speed_test(){

}
