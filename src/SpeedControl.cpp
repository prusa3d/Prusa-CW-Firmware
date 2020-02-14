#include "config.h"
#include "SpeedControl.h"

CSpeedControl::CSpeedControl() : microstep_control(200), washing_speed(10), curing_speed(1),
								 acceleration_flag(false), target_curing_period(220), target_washing_period(200) {}

CSpeedControl::~CSpeedControl() {}

void CSpeedControl::speed_configuration(bool curing_mode) {
	if (curing_mode == true) {
		target_curing_period = map(curing_speed, 1, 10, min_curing_speed, max_curing_speed);
		microstep_control = target_curing_period;
	} else {
		target_washing_period = map(washing_speed, 1, 10, min_washing_speed, max_washing_speed);
		microstep_control = rotation_start;
		acceleration_flag = true;
	}
}

void CSpeedControl::acceleration50ms() {
	if (microstep_control > target_washing_period) {
		if (microstep_control > min_washing_speed + 5)
			microstep_control -= 4;
		microstep_control--;
#ifdef SERIAL_COM_DEBUG
		SerialUSB.print(microstep_control);
		SerialUSB.write('.');
		SerialUSB.print(target_washing_period);
		SerialUSB.write('\n');
#endif
	} else {
		acceleration_flag = false;
	}
}
