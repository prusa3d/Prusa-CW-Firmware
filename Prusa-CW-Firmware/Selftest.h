#pragma once

#ifndef SELFTEST_H
#define SELFTEST_H

/*
1) fukncnost a mereni ventilatoru, alespon minutu aby bezeli.	In progress
2) zkontrolovat cover check ( alespon 5x) otevrit a zavrit		Done
3) to same s ir senzorem pro gastro pan							Done
4) zkontrolovat zda se motor toci pri vsech rychlostech ktere cw1 pouziva (s vlozenou gastro pan)		In progres
5) zkontrolovat heater, alespon at bezi 10minut - zaroven s nim otestovat termistor zda se v zavistlosti zmeni teplota.
6) zkontrolovat jestli bezi ledky, taky alespon 10 minut sviceni.
 */


#include "Countimer.h"

class CSelftest {
public:

	CSelftest();
	~CSelftest();
	void universal_pin_test();
	void ventilation_test(bool f1_error, bool f2_error);
	const char * print();
	void clean_up();
	void measure_state(bool tmp);
	void motor_speed_test();
	void LED_test();
	bool is_first_loop();
	void heat_test(bool heater_error);


	uint8_t phase;
	uint8_t fan1_tacho;		/**< Stores measured rotation per 1 ms for ventilation_test*/
	uint8_t fan2_tacho;		/**< Stores measured rotation per 1 ms for ventilation_test*/
	bool cover_test;
	bool tank_test;
	bool vent_test;
	bool heater_test;
	bool rotation_test;
	bool led_test;
	uint8_t fan1_speed;	// %
	uint8_t fan2_speed;	// %
	Countimer tCountDown;

private:

	bool first_loop;
	bool measured_state;
	bool prev_measured_state;
	uint8_t counter;

};


#endif	//SELFTEST_H
