#pragma once

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

	static void encoder_read();

	static void run_motor();
	static void stop_motor();
	static void speed_configuration(uint8_t speed, bool slow_mode, bool gear_shifting = false);
	static void acceleration();

	static void run_heater();
	static void stop_heater();
	static bool is_heater_running();

	static void run_led();
	static void stop_led();
	static bool is_led_on();

	static bool is_cover_closed();
	static bool is_tank_inserted();

	static void echo();
	static void beep();
	static void warning_beep();

	static void set_fans(uint8_t* duties);
	static void set_target_temp(uint8_t target_temp);
	static void set_fan1_duty(uint8_t duty);
	static void set_fan2_duty(uint8_t duty);

	static bool get_heater_error();
	static uint8_t get_fans_error();

	static Events loop();

	static uint16_t fan_rpm[3];
	static volatile uint8_t fan_tacho_count[3];
	static volatile uint8_t microstep_control;
	static float chamber_temp_celsius;
	static float chamber_temp;
	static float uvled_temp_celsius;
	static float uvled_temp;

private:
	static MCP outputchip;
	static Trinamic_TMC2130 myStepper;

	static void read_adc();
	static int16_t read_adc_raw(uint8_t pin);
	static void fans_duty();
	static void fans_duty(uint8_t fan, uint8_t duty);
	static void fans_PI_regulator();
	static void fans_check();

	static uint8_t lcd_encoder_bits;
	static volatile int8_t rotary_diff;
	static uint8_t target_accel_period;

	static uint8_t fan_duty[3];
	static uint8_t fan_pwm_pins[2];
	static uint8_t fan_enable_pins[2];
	static uint8_t fans_target_temp;

	static uint8_t fan_errors;

	static unsigned long accel_us_last;
	static unsigned long fans_us_last;
	static unsigned long adc_us_last;
	static unsigned long button_timer;
	static double PI_summ_err;
	static bool do_acceleration;
	static bool cover_closed;
	static bool tank_inserted;
	static bool button_active;
	static bool long_press_active;
	static bool heater_error;
	static bool adc_channel;
};

extern Hardware hw;
