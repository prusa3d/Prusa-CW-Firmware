#include "Selftest.h"
#include "i18n.h"

static bool timer_callback_selftest = false;

CSelftest::CSelftest() : phase(0), fan_tacho{0, 0}, cover_test(false), tank_test(false), vent_test(false), heater_test(false),
						 rotation_test(false), led_test(false), fans_speed{10, 10}, cover_down(false),
						 isCounterRunning(false), fail_flag(false), measured_state(false), helper(true), first_loop(true),
						 prev_measured_state(false), counter(0) {
}

CSelftest::~CSelftest() {
}

void CSelftest::tCountDownComplete() {
	timer_callback_selftest = true;
}

bool CSelftest::universal_pin_test() {
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

void CSelftest::ventilation_test(bool fan_error) {
	if (first_loop == true) {
		tCountDown.setCounter(0, 1, 0, tCountDown.COUNT_DOWN, tCountDownComplete);		//fans will do 1 minute
		tCountDown.start();
		first_loop = false;
	}
	if (timer_callback_selftest == false) {
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

bool CSelftest::is_first_loop() {
	return first_loop;
}

const char * CSelftest::print() {
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

void CSelftest::clean_up() {
	first_loop = true;
	counter = 0;
	measured_state = prev_measured_state = false;
	fans_speed[0] = 10;
	fans_speed[1] = fans_speed[0];
	timer_callback_selftest = false;
	helper = false;
	isCounterRunning = false;
	fail_flag = false;
}

void CSelftest::LED_test() {
	if (first_loop == true) {
		tCountDown.setCounter(0, 10, 0, tCountDown.COUNT_DOWN, tCountDownComplete);		//leds will light 10 minutes
		tCountDown.start();
		isCounterRunning = true;
		first_loop = false;
	}
	if (timer_callback_selftest == false) {
		if (tCountDown.isStopped()) {
			tCountDown.start();
			isCounterRunning = true;
		}
	} else {
		isCounterRunning = false;
		led_test = true;
	}
}

bool CSelftest::motor_rotation_timer() {
	if (first_loop) {
		tCountDown.setCounter(0, 0, 10, tCountDown.COUNT_DOWN, tCountDownComplete);			//10sec time periods
		tCountDown.start();
		helper = true;
		return true;
	}

	if (timer_callback_selftest == true) {
		timer_callback_selftest = false;
		tCountDown.restart();
		return true;
	}
	return false;
}
void CSelftest::set_first_loop(const bool tmp) {
	first_loop = tmp;
}

void CSelftest::heat_test(bool heater_error) {
	if (first_loop == true) {
		tCountDown.setCounter(0, 10, 0, tCountDown.COUNT_DOWN, tCountDownComplete);
		tCountDown.start();
		first_loop = false;
		isCounterRunning = true;
	}
	if (timer_callback_selftest == false) {
		if (heater_error) {
			tCountDown.stop();
			fail_flag = true;
			heater_test = true;
			isCounterRunning = false;
		}
	} else {
		heater_test = true;
		isCounterRunning = false;
	}
}
