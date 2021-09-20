#pragma once

#include "hardware.h"
#include "simple_print.h"

class CW1 : public Hardware, public SimplePrint {
public:
	CW1();
	void one_ms_tick();
	char* print_heater_fan(char* buffer, uint8_t size);

private:
	inline bool handle_heater();
	void heating();
	void set_heater_pin_state(bool value);
	inline void set_cooling_speed(uint8_t speed);
	float adjust_chamber_temp(int16_t temp);

	unsigned long heater_on_ms;
	bool heater_pin_state;
};


extern CW1 hw;
