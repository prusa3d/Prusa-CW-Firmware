#ifndef SELFTEST_H
#define SELFTEST_H

/*
1) fukncnost a mereni ventilatoru, alespon minutu aby bezeli.
2) zkontrolovat cover check ( alespon 5x) otevrit a zavrit		Done
3) to same s ir senzorem pro gastro pan							Done
4) zkontrolovat zda se motor toci pri vsech rychlostech ktere cw1 pouziva (s vlozenou gastro pan)		In progres
5) zkontrolovat heater, alespon at bezi 10minut - zaroven s nim otestovat termistor zda se v zavistlosti zmeni teplota.
6) zkontrolovat jestli bezi ledky, taky alespon 10 minut sviceni.
 */

struct selftest_t {

	int phase;
	bool first_loop;
	bool measured_state;
	bool prev_measured_state;
	int counter;
	bool cover_test;
	bool tank_test;
	bool vent_test;
	bool heater_test;
	bool rotation_test;
	bool led_test;

	selftest_t(): phase(0), first_loop(true), measured_state(false), prev_measured_state(false), counter(0) {}

	void universal_pin_test(){
		if(first_loop){
			prev_measured_state = measured_state;
		  	first_loop = false;
		}
		if(measured_state != prev_measured_state){
			prev_measured_state = measured_state;
		  	counter++;
		}
	}

	const char * print(){
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
		default:
			return "";
			break;
		}
	}

	void cleanUp(){
		first_loop = true;
		counter = 0;
		measured_state = prev_measured_state = false;
	}
	void motor_speed_test(){

	}
	void ventilation_test(){

	}

};


#endif
