#include "device.h"
#include "i18n.h"
#include "LiquidCrystal_Prusa.h"
#include "config.h"

CW1S::CW1S()
:
	Hardware(0x3230),	// "02"
	wanted_heater_pin_state(false),
	heater_pin_state(false),
	heater_pwm(0)
{}

void CW1S::one_ms_tick() {
	static uint16_t ticks;	// inicialized with zero by standard
	if (heater_ms_last) {
		// heater is on
		if (++ticks > 1000) {
			ticks = 0;
		}
		wanted_heater_pin_state = ticks < heater_pwm;
	}
}

void CW1S::stop_heater() {
	wanted_heater_pin_state = false;
	set_heater_pin_state(false);
	Hardware::stop_heater();
}

char* CW1S::print_heater_fan(char* buffer, uint8_t size) {
	buffer_init(buffer, size);
	print_P(pgmstr_fan1);
	print(fan_rpm[0]);
	return get_position();
}

bool CW1S::handle_heater() {
	if (wanted_heater_pin_state != heater_pin_state) {
		set_heater_pin_state(wanted_heater_pin_state);
	}
	return fan_rpm[0];
}

void CW1S::heating() {
	if (fans_forced) {
		return;
	}
	if (heater_ms_last) {
		// heater is on
		float error = chamber_target_temp - chamber_temp_celsius;
		uint16_t pwm = error > 0.0 ? round((error < 2.0 ? error : 2.0) * 500) : 0;
		USB_PRINT(error);
		USB_PRINTP("->");
		USB_PRINTLN(pwm);
		heater_pwm = pwm;
		set_fan_speed(0, HEATING_ON_FAN1_SPEED);
	} else {
		// heater is off
		set_fan_speed(0, MIN_FAN_SPEED);
	}
}

void CW1S::set_heater_pin_state(bool value) {
	outputchip.digitalWrite(FAN_HEAT_PIN, value);
	heater_pin_state = value;
}

void CW1S::set_cooling_speed(uint8_t speed) {
	set_fan_speed(1, speed);
}


CW1S hw;
