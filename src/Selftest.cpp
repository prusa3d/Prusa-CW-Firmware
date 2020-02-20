#include "Selftest.h"
#include "i18n.h"

Selftest::Selftest() :
		phase(0),
		fan_tacho{0, 0},
		cover_test(false),
		tank_test(false),
		vent_test(false),
		heater_test(false),
		rotation_test(false),
		led_test(false),
		fans_speed{10, 10},
		tCountDown(Countimer::COUNT_DOWN),
		cover_down(false),
		fail_flag(false),
		measured_state(false),
		helper(true),
		first_loop(true),
		prev_measured_state(false),
		counter(0)
{ }

bool Selftest::universal_pin_test() {
	if (first_loop) {
		prev_measured_state = measured_state;
	  	first_loop = false;
	}
	if (measured_state != prev_measured_state) {
		prev_measured_state = measured_state;
		counter++;
		if (counter > 5 && phase == 1)
			cover_test = true;
		else if (counter > 5 && phase == 2)
			tank_test = true;
		return true;
	}
	return false;
}

void Selftest::ventilation_test(bool fan_error) {
	if (first_loop == true) {
		tCountDown.setCounter(0, 1, 0);		// fans will do 1 minute
		tCountDown.start();
		first_loop = false;
	}
	if (!tCountDown.isCounterCompleted()) {
		uint8_t currSec = tCountDown.getCurrentSeconds();
		if (currSec % 10 == 0) {
			if (fans_speed[0] + 20 <= 100 && helper) {
				fans_speed[0] += 20;
				fans_speed[1] += 20;
				helper = false;
			}
		} else {
			helper = true;
		}

		if (fan_error) {
			tCountDown.stop();
			fans_speed[0] = 10;
			fans_speed[1] = fans_speed[0];
			measured_state = true;			//variable recycling
			prev_measured_state = true;
			fail_flag = true;
			vent_test = true;
		}
	 } else {
		fans_speed[0] = 10;
		fans_speed[1] = fans_speed[0];
		 vent_test = true;
	 }
}

bool Selftest::is_first_loop() {
	return first_loop;
}

const char* Selftest::print() {
	switch (phase) {
	case 3:
		if (!vent_test)
			return pgmstr_fan_test;
		else {
			if (measured_state || prev_measured_state)
				return pgmstr_test_failed;
			else
				return pgmstr_test_success;
		}
	case 4:
		if (!led_test) {
			if (cover_down)
				return pgmstr_led_test;
			else
				return pgmstr_close_cover;
		} else {
			if (fail_flag == true)
				return pgmstr_test_failed;
			else
				return pgmstr_test_success;
		}
		break;
	case 5:
		if (!heater_test) {
			if (fans_speed[0] == 0)
				return pgmstr_remove_tank;
			else {
				if (fans_speed[1] == 0)
					return pgmstr_close_cover;
				else
					return pgmstr_heater_test;
			}
		} else {
			if (fail_flag == true)
				return pgmstr_test_failed;
			else
				return pgmstr_test_success;
		}
		break;

	case 6:
		if (!rotation_test)
			return pgmstr_rotetion_test;
		else
			return pgmstr_test_success;
		break;

	default:
		return pgmstr_emptystr;
		break;
	}
}

void Selftest::clean_up() {
	first_loop = true;
	counter = 0;
	measured_state = prev_measured_state = false;
	fans_speed[0] = 10;
	fans_speed[1] = fans_speed[0];
	tCountDown.stop();
	helper = false;
	fail_flag = false;
}

void Selftest::LED_test() {
	if (first_loop == true) {
		tCountDown.setCounter(0, 10, 0);		// leds will light 10 minutes
		tCountDown.start();
		first_loop = false;
	}
	if (!tCountDown.isCounterCompleted()) {
		if (tCountDown.isStopped()) {
			tCountDown.start();
		}
	} else {
		led_test = true;
	}
}

bool Selftest::motor_rotation_timer() {
	if (first_loop) {
		tCountDown.setCounter(0, 0, 10);			// 10sec time periods
		tCountDown.start();
		helper = true;
		return true;
	}

	if (tCountDown.isCounterCompleted()) {
		tCountDown.restart();
		return true;
	}
	return false;
}

void Selftest::set_first_loop(const bool tmp) {
	first_loop = tmp;
}

void Selftest::heat_test(bool heater_error) {
	if (first_loop == true) {
		tCountDown.setCounter(0, 10, 0);
		tCountDown.start();
		first_loop = false;
	}
	if (!tCountDown.isCounterCompleted()) {
		if (heater_error) {
			tCountDown.stop();
			fail_flag = true;
			heater_test = true;
		}
	} else {
		heater_test = true;
	}
}
