#pragma once

#include "config.h"
#include "hardware.h"


class Speed_Control {
public:
	Speed_Control(hardware& hw, eeprom_v2_t& config);
	void speed_configuration(bool curing_mode);
	void acceleration();

	uint8_t microstep_control;

private:
	hardware& hw;
	eeprom_v2_t& config;
	uint8_t target_washing_period;
	bool do_acceleration;
	unsigned long us_last;
};
