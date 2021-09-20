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
	inline bool handle_heater();
	void heating();
	inline void set_cooling_speed(uint8_t speed);
	void set_heater_pin_state(bool value);
	float adjust_chamber_temp(int16_t temp);

	bool wanted_heater_pin_state;
	bool heater_pin_state;
	uint16_t heater_pwm;
};


extern CW1S hw;
