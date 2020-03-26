#pragma once

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

class Hardware {
public:
	Hardware();

	void encoder_read();

	void run_motor();
	void stop_motor();
	void speed_configuration(bool curing_mode);
	void acceleration();

	void run_heater();
	void stop_heater();
	bool is_heater_running();

	void run_led();
	void stop_led();
	bool is_led_on();

	bool is_cover_closed();
	bool is_tank_inserted();

	void echo();
	void beep();
	void warning_beep();

	void set_fans(uint8_t* duties);
	void set_target_temp(uint8_t target_temp);
	bool get_heater_error();
	uint8_t get_fans_error();

	Events loop();

	volatile int fan_tacho_count[3];
	volatile uint8_t microstep_control;
	float chamber_temp;

private:
	thermistor therm1;
	MCP outputchip;
	Trinamic_TMC2130 myStepper;

	float therm1_read();
	void fans_duty();
	void fans_PI_regulator();
	void fans_check();

	uint8_t lcd_encoder_bits;
	volatile int8_t rotary_diff;
	uint8_t target_accel_period;

	uint8_t fan_duty[3];
	uint8_t fan_pwm_pins[2];
	uint8_t fan_enable_pins[2];
	uint8_t fans_target_temp;

	int fan_tacho_last_count[3];
	uint8_t fan_errors;

	unsigned long accel_us_last;
	unsigned long fans_us_last;
	unsigned long therm_us_last;
	unsigned long button_timer;
	double PI_summ_err;
	bool do_acceleration;
	bool cover_closed;
	bool tank_inserted;
	bool button_active;
	bool long_press_active;
	bool heater_error;
};

extern Hardware hw;
