#pragma once

#include "Arduino.h"
#include "thermistor.h"
#include "MCP23S17.h"
#include "Trinamic_TMC2130.h"
#include "defines.h"

float celsius2fahrenheit(float);
float fahrenheit2celsius(float);

class hardware {
public:
	hardware();
	~hardware();

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

	void read_encoder(volatile uint8_t&);
	bool is_button_pressed();

	void set_fans_duty(uint8_t*);
	bool fan_rpm();
	bool get_heater_error();
	uint8_t get_fans_error();

	volatile int fan_tacho_count[3];

private:
	thermistor therm1;
	MCP outputchip;
	Trinamic_TMC2130 myStepper;

	uint8_t lcd_encoder_bits;

	uint8_t fans_duty[3];
	uint8_t fans_pwm_pins[2];
	uint8_t fans_enable_pins[2];

	int rpm_fan_counter;
	int fan_tacho_last_count[3];
	bool fan_error[3];
};
