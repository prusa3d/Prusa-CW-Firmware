#pragma once

#include "Arduino.h"
#include "thermistor.h"
#include "MCP23S17.h"
#include "Trinamic_TMC2130.h"
#include "defines.h"

float celsius2fahrenheit(float);
float fahrenheit2celsius(float);

struct __attribute__((__packed__)) Events {
	bool cover_opened;
	bool cover_closed;
	bool tank_inserted;
	bool tank_removed;
	bool button_short_press;
	bool button_long_press;
	bool control_up;
	bool control_down;
};

class hardware {
public:
	hardware();

	float therm1_read();
	void run_motor();
	void stop_motor();
	void motor_configuration(bool);
	void motor_noaccel_settings();

	void run_heater();
	void stop_heater();
	bool is_heater_running();

	void run_led(uint8_t);
	void stop_led();
	bool is_led_on();

	bool is_cover_closed();
	bool is_tank_inserted();

	void echo();
	void beep();
	void warning_beep();

	void read_encoder();

	void set_fans_duty(uint8_t*);
	void fan_rpm();
	bool get_heater_error();
	uint8_t get_fans_error();

	Events get_events(bool sound_response);

	volatile int fan_tacho_count[3];

private:
	thermistor therm1;
	MCP outputchip;
	Trinamic_TMC2130 myStepper;

	uint8_t lcd_encoder_bits;
	volatile int8_t rotary_diff;

	uint8_t fans_duty[3];
	uint8_t fans_pwm_pins[2];
	uint8_t fans_enable_pins[2];

	int fan_tacho_last_count[3];
	uint16_t rpm_fan_counter;
	uint8_t fan_errors;

	unsigned long button_timer;
	bool cover_closed;
	bool tank_inserted;
	bool button_active;
	bool long_press_active;
};
