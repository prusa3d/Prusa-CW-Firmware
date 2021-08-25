#pragma once

#include "MCP23S17.h"
#include "Trinamic_TMC2130.h"
#include "defines.h"

#define EVENT_COVER_OPENED			1
#define EVENT_COVER_CLOSED			2
#define EVENT_TANK_INSERTED			4
#define EVENT_TANK_REMOVED			8
#define EVENT_BUTTON_SHORT_PRESS	16
#define EVENT_BUTTON_LONG_PRESS		32
#define EVENT_CONTROL_UP			64
#define EVENT_CONTROL_DOWN			128

float celsius2fahrenheit(float);
float fahrenheit2celsius(float);

class Hardware {
public:
	Hardware(uint16_t model_magic);

	void encoder_read();
	void run_motor();
	void stop_motor();
	void enable_stepper();
	void disable_stepper();
	void speed_configuration(uint8_t speed, bool fast_mode, bool gear_shifting = false);
	void acceleration();

	void run_heater();
	void stop_heater();
	void run_led();
	void stop_led();

	bool is_cover_closed();
	bool is_tank_inserted();

	void echo();
	void beep();
	void warning_beep();

	void set_chamber_target_temp(uint8_t target_temp);

	void set_fans(uint8_t* duties);
	void fans_duty(uint8_t fan, uint8_t duty);

	uint8_t loop();

	uint16_t model_magic;
	uint16_t fan_rpm[3];
	volatile uint8_t fan_tacho_count[3];
	volatile uint8_t microstep_control;
	float chamber_temp_celsius;
	float chamber_temp;
	float uvled_temp_celsius;
	float uvled_temp;
	bool heater_error;

protected:
	MCP outputchip;
	Trinamic_TMC2130 stepper;

	void read_adc();
	int16_t read_adc_raw(uint8_t pin);
	void fans_duty();
	void fans_PI_regulator();
	void fans_rpm();
	virtual void heat_control();
	virtual bool handle_heater();

	uint8_t lcd_encoder_bits;
	volatile int8_t rotary_diff;
	uint8_t target_accel_period;

	uint8_t fan_duty[2];
	uint8_t chamber_target_temp;

	unsigned long accel_ms_last;
	unsigned long one_second_ms_last;
	unsigned long heater_ms_last;
	unsigned long button_timer;
	double PI_summ_err;
	bool do_acceleration;
	bool cover_closed;
	bool tank_inserted;
	bool button_active;
	bool long_press_active;
	bool adc_channel;
};
