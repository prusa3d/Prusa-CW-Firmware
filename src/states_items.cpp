#include "states.h"

namespace States {

	// shared counter for all states (RAM saver)
	Countimer timer;

	// States::Base
	Base::Base(
		const char* title,
		uint8_t options,
		uint8_t* fans_duties,
		Base* continue_to,
		uint8_t* continue_after,
		uint8_t* motor_speed,
		uint8_t* target_temp)
	:
		continue_to(continue_to),
		message(nullptr),
		target_temp(target_temp),
		fans_duties(fans_duties),
		us_last(0),
		canceled(false),
		title(title),
		continue_after(continue_after),
		motor_speed(motor_speed),
		options(options)
	{}

	void Base::start() {
		canceled = false;
		hw.set_fans(fans_duties);
		if (continue_after) {
			timer.setCounter(0, *continue_after, 0, options & STATE_OPTION_TIMER_UP);
		}
		/* FIXME PI_regulator is not working as expected
		if (target_temp) {
			hw.set_target_temp(*target_temp);
		}
		*/
		if (options & (STATE_OPTION_UVLED | STATE_OPTION_HEATER) && (!hw.is_cover_closed() || hw.is_tank_inserted())) {
			if (continue_after) {
				timer.start();
			}
			do_pause();
		} else {
			do_continue();
		}
	}

	Base* Base::loop() {
		if (continue_after) {
			timer.run();
		}
		if (canceled || (continue_after && timer.isCounterCompleted())) {
			return continue_to;
		}
		if (hw.heater_error) {
			error.new_text(pgmstr_heater_error, pgmstr_please_restart);
			return &error;
		}
		if (options & STATE_OPTION_UVLED) {
			if (hw.uvled_temp_celsius < 0.0) {
				error.new_text(pgmstr_led_failure, pgmstr_read_temp_error);
				return &error;
			}
			if (hw.uvled_temp_celsius > UVLED_MAX_TEMP) {
				error.new_text(pgmstr_led_failure, pgmstr_overheat_error);
				return &error;
			}
		}
		if (us_last && millis() - us_last > LED_DELAY && options & STATE_OPTION_UVLED) {
			hw.run_led();
			us_last = 0;
		}
		return nullptr;
	}

	bool Base::get_info1(__attribute__((unused)) char* buffer, __attribute__((unused)) uint8_t size) {
		return false;
	}

	bool Base::get_info2(__attribute__((unused)) char* buffer, __attribute__((unused)) uint8_t size) {
		return false;
	}

	void Base::do_pause() {
		if (options & STATE_OPTION_UVLED) {
			hw.stop_led();
		}
		if (options & STATE_OPTION_HEATER) {
			hw.stop_heater();
		}
		if (motor_speed) {
			hw.stop_motor();
		}
		if (continue_after) {
			timer.pause();
		}
	}

	void Base::do_continue() {
		if (motor_speed) {
			hw.speed_configuration(*motor_speed, options & STATE_OPTION_WASHING);
			hw.run_motor();
		}
		if (options & STATE_OPTION_HEATER) {
			hw.run_heater();
		}
		if (continue_after) {
			timer.start();
		}
		us_last = millis();
	}

	void Base::pause_continue() {
		if (continue_after) {
			if (timer.isStopped()) {
				if (!get_hw_pause_reason()) {
					do_continue();
				}
			} else {
				do_pause();
			}
		}
	}

	void Base::cancel() {
		canceled = true;
	}

	void Base::process_events(uint8_t events) {
		if ((options & STATE_OPTION_WASHING && events & EVENT_TANK_REMOVED) || (options & (STATE_OPTION_UVLED | STATE_OPTION_HEATER) && events & EVENT_COVER_OPENED)) {
			do_pause();
		}
		if (options & (STATE_OPTION_UVLED | STATE_OPTION_HEATER) && events & EVENT_COVER_CLOSED && !hw.is_tank_inserted()) {
			do_continue();
		}
	}

	bool Base::short_press_cancel() {
		if (options & STATE_OPTION_SHORT_CANCEL) {
			return true;
		}
		return false;
	}

	const char* Base::get_title() {
		if (continue_after && timer.isStopped()) {
			const char* pause_reason = get_hw_pause_reason();
			return pause_reason ? pause_reason : pgmstr_paused;
		}
		return title;
	}

	const char* Base::get_message() {
		return message;
	}

	uint16_t Base::get_time() {
		if (continue_after) {
			return timer.getCurrentTimeInSeconds();
		}
		return UINT16_MAX;
	}

	float Base::get_temperature() {
		if (options & STATE_OPTION_CHAMB_TEMP) {
			return hw.chamber_temp;
		}
		if (options & STATE_OPTION_UVLED_TEMP) {
			return hw.uvled_temp;
		}
		return -40.0;
	}

	const char* Base::decrease_time() {
		if (continue_after && options & STATE_OPTION_CONTROLS) {
			uint16_t secs = timer.getCurrentTimeInSeconds();
			if (secs < INC_DEC_TIME_STEP) {
				return pgmstr_min_symb;
			} else {
				timer.setCounterInSeconds(secs - INC_DEC_TIME_STEP);
				return pgmstr_double_lt;
			}
		}
		return nullptr;
	}

	const char* Base::increase_time() {
		if (continue_after && options & STATE_OPTION_CONTROLS) {
			uint16_t secs = timer.getCurrentTimeInSeconds();
			if (secs > 10 * 60 - INC_DEC_TIME_STEP) {
				return pgmstr_max_symb;
			} else {
				timer.setCounterInSeconds(secs + INC_DEC_TIME_STEP);
				return pgmstr_double_gt;
			}
		}
		return nullptr;
	}

	bool Base::is_paused() {
		if (continue_after) {
			return timer.isStopped();
		}
		return false;
	}

	bool Base::is_finished() {
		if (options & STATE_OPTION_SHORT_CANCEL) {
			return canceled;
		}
		return false;
	}

	void Base::set_continue_to(Base* to) {
		continue_to = to;
	}

	void Base::new_text(const char* new_title, const char* new_message) {
		title = new_title;
		message = new_message;
	}

	const char* Base::get_hw_pause_reason() {
		if (options & STATE_OPTION_WASHING) {
			if (!hw.is_tank_inserted()) {
				return pgmstr_insert_tank;
			}
		} else {
			if (hw.is_tank_inserted()) {
				return pgmstr_remove_tank;
			}
			if (!hw.is_cover_closed()) {
				return pgmstr_close_cover;
			}
		}
		return nullptr;
	}


	// States::Warmup
	Warmup::Warmup(
		const char* title,
		Base* continue_to,
		uint8_t* continue_after,
		uint8_t* motor_speed,
		uint8_t* target_temp)
	:
		Base(title, STATE_OPTION_TIMER_UP | STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP, config.fans_drying_speed, continue_to, continue_after, motor_speed, target_temp)
	{}

	Base* Warmup::loop() {
		if (!config.heat_to_target_temp || hw.chamber_temp >= *target_temp) {
			return continue_to;
		}
		return Base::loop();
	}


	// States::Confirm
	Confirm::Confirm() :
		Base(pgmstr_emptystr, STATE_OPTION_SHORT_CANCEL), quit(false)
	{}

	void Confirm::start() {
		canceled = false;
		quit = true;
		us_last = 1;				// beep
		const char* text2 = pgmstr_emptystr;
		switch(config.finish_beep_mode) {
			case 2:
				quit = false;
				text2 = pgmstr_press2continue;
				break;
			case 0:
				us_last = 0;		// no beep
				break;
			default:
				break;
		}
		confirm.new_text(pgmstr_finished, text2);
		hw.set_fans(fans_duties);
	}

	Base* Confirm::loop() {
		canceled = quit;
		unsigned long us_now = millis();
		if (us_last && us_now - us_last > 1000) {
			hw.beep();
			us_last = us_now;
		}
		if (canceled) {
			return continue_to;
		} else {
			return nullptr;
		}
	}


	// States::Test_switch
	Test_switch::Test_switch(
		const char* title,
		Base* continue_to,
		const char* message_on,
		const char* message_off,
		bool (*value_getter)())
	:
		Base(title, 0, config.fans_menu_speed, continue_to),
		message_on(message_on),
		message_off(message_off),
		value_getter(value_getter),
		test_count(0),
		old_state(false)
	{}

	void Test_switch::start() {
		canceled = false;
		old_state = value_getter();
		message = old_state ? message_on : message_off;
		test_count = SWITCH_TEST_COUNT;
		us_last = millis();
	}

	Base* Test_switch::loop() {
		if (canceled || !test_count) {
			hw.beep();
			return continue_to;
		}
		unsigned long us_now = millis();
		if (us_now - us_last > 250) {
			us_last = us_now;
			bool state = value_getter();
			if (old_state != state) {
				old_state = state;
				message = old_state ? message_on : message_off;
				--test_count;
			}
		}
		return nullptr;
	}


	// States::Test_rotation
	Test_rotation::Test_rotation(
		const char* title,
		Base* continue_to)
	:
		Base(title, 0, config.fans_menu_speed, continue_to, &test_time, &test_speed),
		test_time(ROTATION_TEST_TIME),
		test_speed(0),
		old_seconds(0),
		fast_mode(false),
		draw(false)
	{}

	void Test_rotation::start() {
		test_speed = 10;
		old_seconds = 60 * ROTATION_TEST_TIME;
		fast_mode = true;
		draw = true;
		Base::start();
		hw.speed_configuration(test_speed, fast_mode);
	}

	Base* Test_rotation::loop() {
		uint16_t seconds = timer.getCurrentTimeInSeconds();
		if (seconds != old_seconds) {
			old_seconds = seconds;
			if (seconds && !(seconds % (60 * ROTATION_TEST_TIME / 20))) {
				if (!(--test_speed)) {
					test_speed = 10;
					fast_mode = false;
				}
				hw.speed_configuration(test_speed, fast_mode, true);
				draw = true;
			}
		}
		return Base::loop();
	}

	bool Test_rotation::get_info1(char* buffer, uint8_t size) {
		if (draw) {
			buffer[0] = fast_mode ? 'W' : 'C';
			buffer_init(++buffer, --size);
			print(test_speed, 10, '0');
			get_position()[0] = char(0);
			draw = false;
			return true;
		}
		return false;
	}


	// States::Test_fans
	Test_fans::Test_fans(
		const char* title,
		Base* continue_to)
	:
		Base(title, 0, fans_speed, continue_to, &test_time),
		test_time(FANS_TEST_TIME),
		fans_speed{0, 0},
		old_fan_rpm{0, 0},
		old_seconds(0),
		draw1(false),
		draw2(false)
	{}

	void Test_fans::start() {
		fans_speed[0] = 0;
		fans_speed[1] = 100;
		old_fan_rpm[0] = 0;
		old_fan_rpm[1] = UINT16_MAX;
		old_seconds = 60 * FANS_TEST_TIME;
		draw1 = true;
		draw2 = true;
		Base::start();
	}

	Base* Test_fans::loop() {
		uint16_t seconds = timer.getCurrentTimeInSeconds();
		if (seconds != old_seconds) {
			old_seconds = seconds;
			draw2 = true;
			if (seconds && !((seconds - 1) % (60 * FANS_TEST_TIME / 6))) {
				if (!fans_speed[0] && hw.fan_rpm[0]) {
					error.new_text(pgmstr_fan1_failure, pgmstr_spinning);
					return &error;
				}
				if (fans_speed[0] && hw.fan_rpm[0] <= old_fan_rpm[0]) {
					error.new_text(pgmstr_fan1_failure, pgmstr_not_spinning);
					return &error;
				}
				if (!fans_speed[1] && hw.fan_rpm[1]) {
					error.new_text(pgmstr_fan2_failure, pgmstr_spinning);
					return &error;
				}
				if (fans_speed[1] && hw.fan_rpm[1] >= old_fan_rpm[1]) {
					error.new_text(pgmstr_fan2_failure, pgmstr_not_spinning);
					return &error;
				}
				if (fans_speed[0] < 100) {
					fans_speed[0] += 20;
					fans_speed[1] = 100 - fans_speed[0];
					hw.set_fans(fans_speed);
					draw1 = true;
					old_fan_rpm[0] = hw.fan_rpm[0];
					old_fan_rpm[1] = hw.fan_rpm[1];
				}
			}
		}
		return Base::loop();
	}

	bool Test_fans::get_info1(char* buffer, uint8_t size) {
		if (draw1) {
			buffer_init(buffer, size);
			print(fans_speed[0]);
			print_P(pgmstr_double_space+1);
			print(fans_speed[1]);
			get_position()[0] = char(0);
			draw1 = false;
			return true;
		}
		return false;
	}

	bool Test_fans::get_info2(char* buffer, uint8_t size) {
		if (draw2) {
			buffer_init(buffer, size);
			print_P(pgmstr_fan1);
			print(hw.fan_rpm[0]);
			print_P(pgmstr_fan2);
			print(hw.fan_rpm[1]);
			get_position()[0] = char(0);
			draw2 = false;
			return true;
		}
		return false;
	}


	// States::Test_uvled
	Test_uvled::Test_uvled(
		const char* title,
		uint8_t* fans_duties,
		Base* continue_to)
	:
		Base(title, STATE_OPTION_UVLED | STATE_OPTION_UVLED_TEMP, fans_duties, continue_to, &test_time),
		test_time(UVLED_TEST_TIME),
		old_uvled_temp(0.0)
	{}

	void Test_uvled::start() {
		old_uvled_temp = hw.uvled_temp_celsius;
		Base::start();
	}

	Base* Test_uvled::loop() {
		uint16_t seconds = timer.getCurrentTimeInSeconds() - 1;
		if (!seconds && old_uvled_temp + UVLED_TEST_GAIN > hw.uvled_temp_celsius) {
			error.new_text(pgmstr_led_failure, pgmstr_nopower_error);
			return &error;
		}
		return Base::loop();
	}


	// States::Test_heater
	Test_heater::Test_heater(
		const char* title,
		uint8_t* fans_duties,
		Base* continue_to)
	:
		Base(title, STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP, fans_duties, continue_to, &test_time),
		test_time(HEATER_TEST_TIME),
		old_chamb_temp(0.0),
		old_seconds(0),
		draw(false)
	{}

	void Test_heater::start() {
		old_chamb_temp = hw.chamber_temp_celsius;
		draw = true;
		Base::start();
	}

	Base* Test_heater::loop() {
		if (old_chamb_temp < 0.0 || hw.chamber_temp_celsius < 0.0) {
			error.new_text(pgmstr_heater_failure, pgmstr_read_temp_error);
			return &error;
		}
		uint16_t seconds = timer.getCurrentTimeInSeconds() - 1;
		if (!seconds && old_chamb_temp + HEATER_TEST_GAIN > hw.chamber_temp_celsius) {
			error.new_text(pgmstr_heater_failure, pgmstr_nopower_error);
			return &error;
		}
		if (seconds != old_seconds) {
			old_seconds = seconds;
			draw = true;
		}
		return Base::loop();
	}

	bool Test_heater::get_info2(char* buffer, uint8_t size) {
		if (draw) {
			buffer_init(buffer, size);
			print_P(pgmstr_fan3);
			print(hw.fan_rpm[2]);
			get_position()[0] = char(0);
			draw = false;
			return true;
		}
		return false;
	}

}
