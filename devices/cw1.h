#pragma once

#include "hardware.h"
#include "simple_print.h"

class CW1 : public Hardware, public SimplePrint {
public:
	CW1();
	void one_ms_tick();
	void run_heater();
	void stop_heater();
	char* print_heater_fan(char* buffer, uint8_t size);

private:
	inline bool handle_heater();
	inline void heating();
	inline void set_cooling_speed(uint8_t speed);
};


extern CW1 hw;
