#include "device.h"
#include "i18n.h"
#include "LiquidCrystal_Prusa.h"

CW1::CW1()
:
	Hardware(0x3130),	// "01"
	heater_on_ms(0),
	heater_pin_state(false)
{}

void CW1::one_ms_tick() {
}

char* CW1::print_heater_fan(char* buffer, uint8_t size) {
	buffer_init(buffer, size);
	print_P(pgmstr_fan3);
	print(fan_rpm[2]);
	return get_position();
}

inline bool CW1::handle_heater() {
	return heater_pin_state && !fan_rpm[2] && millis() - heater_on_ms > HEATER_CHECK_DELAY;
}

void CW1::heating() {
	if (heating_in_progress and round_short(get_configured_temp(chamber_temp_celsius)) < chamber_target_temp) {
		set_heater_pin_state(true);
	} else {
		set_heater_pin_state(false);
	}
}

void CW1::set_heater_pin_state(bool value) {
	if (heater_pin_state != value) {
		USB_PRINTP("heater: ");
		USB_PRINTLN(value);
		outputchip.digitalWrite(FAN_HEAT_PIN, value);
		heater_pin_state = value;
		if (value) {
			heater_on_ms = millis();
		}
	}
}

inline void CW1::set_cooling_speed(uint8_t speed) {
	set_fan_speed(0, speed);
	set_fan_speed(1, speed);
}

float CW1::adjust_chamber_temp(int16_t temp) {
//	USB_PRINTLN(temp);
	if (temp < 0.0) {
		return temp;
	}
	// approx. dependency of chamber temp (2 mins delay)
	unsigned long ms_now = millis();
	if (heating_in_progress && ms_now - heating_started_ms > 120000) {
		return 0.15 * temp;
	}
	return 0.0;
}

CW1 hw;
