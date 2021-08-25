#include "device.h"
#include "i18n.h"
#include "LiquidCrystal_Prusa.h"

CW1::CW1()
:
	Hardware(0x3130)	// "01"
{}

void CW1::one_ms_tick() {
}

void CW1::run_heater() {
	outputchip.digitalWrite(FAN_HEAT_PIN, HIGH);
	Hardware::run_heater();
}

void CW1::stop_heater() {
	outputchip.digitalWrite(FAN_HEAT_PIN, LOW);
	Hardware::stop_heater();
}

char* CW1::print_heater_fan(char* buffer, uint8_t size) {
	buffer_init(buffer, size);
	print_P(pgmstr_fan3);
	print(fan_rpm[2]);
	return get_position();
}

inline bool CW1::handle_heater() {
	return fan_rpm[2];
}

void CW1::heat_control() {
	// TODO turn heater on and off based on chamber temperature
}


CW1 hw;
