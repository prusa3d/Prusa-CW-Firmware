#pragma once

#ifndef SELFTEST_H
#define SELFTEST_H

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
