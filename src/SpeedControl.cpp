#include "defines.h"
#include "SpeedControl.h"
#include "hardware.h"

Speed_Control::Speed_Control(hardware& hw, eeprom_v2_t& config) :
		microstep_control(WASHING_ROTATION_START),
		hw(hw),
		config(config),
		target_washing_period(WASHING_ROTATION_START),
		do_acceleration(false),
		us_last(0)
{ }

void Speed_Control::speed_configuration(bool curing_mode) {
	hw.motor_configuration(curing_mode);
	if (curing_mode) {
		microstep_control = map(config.curing_speed, 1, 10, MIN_CURING_SPEED, MAX_CURING_SPEED);
	} else {
		target_washing_period = map(config.washing_speed, 1, 10, MIN_WASHING_SPEED, MAX_WASHING_SPEED);
		microstep_control = WASHING_ROTATION_START;
		us_last = millis();
	}
	do_acceleration = !curing_mode;
}

void Speed_Control::acceleration() {
	if (!do_acceleration)
		return;

	unsigned long us_now = millis();
	if (us_now - us_last < 50)
		return;

	us_last = us_now;

	if (microstep_control > target_washing_period) {
		// step is 5 to MIN_WASHING_SPEED+5, then step is 1
		if (microstep_control > MIN_WASHING_SPEED + 5)
			microstep_control -= 4;
		microstep_control--;
#ifdef SERIAL_COM_DEBUG
		SerialUSB.print(microstep_control);
		SerialUSB.print("->");
		SerialUSB.print(target_washing_period);
		SerialUSB.print("\r\n");
#endif
	} else {
		do_acceleration = false;
		hw.motor_noaccel_settings();
	}
}
