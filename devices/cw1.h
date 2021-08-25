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
	inline void heat_control();
};

extern CW1 hw;
