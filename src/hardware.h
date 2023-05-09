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

float celsius2fahrenheit(float celsius);
float fahrenheit2celsius(float fahrenheit);
uint8_t round_short(float);
float get_configured_temp(float temp);


class Hardware {
public:
	Hardware(uint16_t model_magic);

	void encoder_read();
	void run_motor(bool direction);
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
	void force_fan_speed(uint8_t fan_speed_1, uint8_t fan_speed_2);

	uint8_t loop();

	uint16_t model_magic;
	uint16_t fan_rpm[3];
	volatile uint8_t fan_tacho_count[3];
	volatile uint8_t microstep_control;
	float chamber_temp_celsius;
	float uvled_temp_celsius;
	bool heater_error;
	bool disable_controls;

protected:
	MCP outputchip;
	Trinamic_TMC2130 stepper;

	void read_adc();
	int16_t read_adc_raw(uint8_t pin);
	void cooling();
	void fans_rpm();
	void set_fan_speed(uint8_t fan, uint8_t speed);
	virtual void heating() = 0;
	virtual bool handle_heater() = 0;
	virtual void set_cooling_speed(uint8_t speed) = 0;
	virtual float adjust_chamber_temp(int16_t temp) = 0;

	uint8_t lcd_encoder_bits;
	volatile int8_t rotary_diff;
	uint8_t target_accel_period;

	uint8_t fan_speed[2];
	uint8_t chamber_target_temp;

	unsigned long accel_ms_last;
	unsigned long one_second_ms_last;
	unsigned long heating_started_ms;
	unsigned long button_timer;
	bool heating_in_progress;
	bool do_acceleration;
	bool cover_closed;
	bool tank_inserted;
	bool button_active;
	bool long_press_active;
	bool adc_channel;
	bool fans_forced;
};
