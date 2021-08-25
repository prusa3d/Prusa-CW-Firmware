#pragma once

#include "hardware.h"
#include "simple_print.h"

class CW1S : public Hardware, public SimplePrint {
public:
	CW1S();
	void one_ms_tick();
	void stop_heater();
	char* print_heater_fan(char* buffer, uint8_t size);

private:
	bool handle_heater();
	void heat_control();
	void adjust_fan_speed(uint8_t fan, uint8_t duty);
	void set_heater_pin_state(bool value);

	bool wanted_heater_pin_state;
	bool heater_on;
	bool heater_pin_state;
	uint16_t heater_pwm_duty;
};


extern CW1S hw;
