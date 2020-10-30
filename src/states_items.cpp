#include "states.h"

namespace States {

	// shared counter for all states (RAM saver)
	Countimer timer;

	// States::Base
	Base::Base(
		const char* title,
		uint8_t options,
		Base* continue_to,
		uint8_t* continue_after,
		uint8_t* motor_speed,
		uint8_t* target_temp)
	:
		options(options),
		continue_to(continue_to),
		message(nullptr),
		target_temp(target_temp),
		motor_speed(motor_speed),
		continue_after(continue_after),
		title(title)
	{}

	void Base::start(bool handle_heater) {
		ms_last = 0;
		canceled = false;
		motor_direction = false;
		if (continue_after) {
			timer.setCounter(0, *continue_after, 0, options & STATE_OPTION_TIMER_UP);
		}
		if (target_temp) {
			hw.set_chamber_target_temp(*target_temp);
		}
		if (options & (STATE_OPTION_UVLED | STATE_OPTION_HEATER) && (!hw.is_cover_closed() || hw.is_tank_inserted())) {
			if (continue_after) {
				timer.start();
			}
			do_pause(handle_heater);
		} else {
			do_continue(handle_heater);
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
			error.new_text(pgmstr_heater_error, pgmstr_not_spinning);
			return &error;
		}
		if (hw.chamber_temp_celsius < 0.0) {
			error.new_text(pgmstr_heater_failure, pgmstr_read_temp_error);
			return &error;
		}
		if (hw.uvled_temp_celsius < 0.0) {
			error.new_text(pgmstr_led_failure, pgmstr_read_temp_error);
			return &error;
		}
		if (options & STATE_OPTION_UVLED && (uint8_t)hw.uvled_temp_celsius > UVLED_MAX_TEMP) {
			error.new_text(pgmstr_led_failure, pgmstr_overheat_error);
			return &error;
		}
		if (ms_last && millis() - ms_last > LED_DELAY && options & STATE_OPTION_UVLED) {
			hw.run_led();
			ms_last = 0;
		}
		return nullptr;
	}

	bool Base::get_info1(__attribute__((unused)) char* buffer, __attribute__((unused)) uint8_t size) {
		return false;
	}

	bool Base::get_info2(__attribute__((unused)) char* buffer, __attribute__((unused)) uint8_t size) {
		return false;
	}

	void Base::do_pause(bool handle_heater) {
		if (options & STATE_OPTION_UVLED) {
			hw.stop_led();
		}
		if (handle_heater && options & STATE_OPTION_HEATER) {
			hw.stop_heater();
		}
		if (motor_speed) {
			hw.stop_motor();
		}
		if (continue_after) {
			timer.pause();
		}
	}

	void Base::do_continue(bool handle_heater) {
		if (motor_speed) {
			hw.speed_configuration(*motor_speed, options & STATE_OPTION_WASHING);
			hw.run_motor(motor_direction);
		}
		if (handle_heater && options & STATE_OPTION_HEATER) {
			hw.run_heater();
		}
		if (continue_after) {
			timer.start();
		}
		ms_last = millis();
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
			return hw.chamber_temp_celsius;
		}
		if (options & STATE_OPTION_UVLED_TEMP) {
			return hw.uvled_temp_celsius;
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


	// States::Direction_change
	Direction_change::Direction_change(
		const char* title,
		uint8_t options,
		uint8_t* direction_cycles,
		Base* continue_to,
		uint8_t* continue_after,
		uint8_t* motor_speed,
		uint8_t* target_temp)
	:
		Base(title, options, continue_to, continue_after, motor_speed, target_temp),
		direction_cycles(direction_cycles)
	{}

	void Direction_change::start(bool handle_heater) {
		old_seconds = 0;
		stop_seconds = 0;
		if (*direction_cycles) {
			direction_change_time = *continue_after * 60 / *direction_cycles;
		} else {
			direction_change_time = *continue_after * 60;
		}
		Base::start(handle_heater);
	}

	Base* Direction_change::loop() {
		uint16_t seconds = timer.getCurrentTimeInSeconds();
		if (seconds != old_seconds) {
			if (stop_seconds && stop_seconds - seconds >= DIR_CHANGE_DELAY) {
				hw.speed_configuration(*motor_speed, options & STATE_OPTION_WASHING);
				hw.run_motor(motor_direction);
				stop_seconds = 0;
			}
			if (old_seconds && !(seconds % direction_change_time)) {
				hw.stop_motor();
				motor_direction = !motor_direction;
				stop_seconds = seconds;
			}
			old_seconds = seconds;
		}
		return Base::loop();
	}


	// States::Warmup
	Warmup::Warmup(
		const char* title,
		Base* continue_to,
		uint8_t* continue_after,
		uint8_t* motor_speed,
		uint8_t* target_temp)
	:
		Base(title, STATE_OPTION_TIMER_UP | STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP, continue_to, continue_after, motor_speed, target_temp)
	{}

	Base* Warmup::loop() {
		if (!config.heat_to_target_temp || round_short(get_configured_temp(hw.chamber_temp_celsius)) >= *target_temp) {
			return continue_to;
		}
		return Base::loop();
	}


	// States::Cooldown
	Cooldown::Cooldown(Base* continue_to) :
		Base(pgmstr_cooldown, STATE_OPTION_CONTROLS, continue_to, &cooldown_time),
		cooldown_time(COOLDOWN_RUNTIME)
	{}

	void Cooldown::start(bool handle_heater) {
		hw.force_fan_speed(100, 100);
		Base::start(handle_heater);
	}


	// States::Confirm
	Confirm::Confirm(bool force_wait) :
		Base(pgmstr_emptystr, STATE_OPTION_SHORT_CANCEL), force_wait(force_wait), quit(false)
	{}

	void Confirm::start(__attribute__((unused)) bool handle_heater) {
		canceled = false;
		quit = true;
		ms_last = 1;				// beep
		const char* text2 = pgmstr_emptystr;
		uint8_t mode = config.finish_beep_mode;
		if (force_wait) {
			mode = 2;
			hw.disable_controls = true;
		}
		switch(mode) {
			case 2:
				quit = false;
				text2 = pgmstr_press2continue;
				break;
			case 0:
				ms_last = 0;		// no beep
				break;
			default:
				break;
		}
		confirm.new_text(pgmstr_finished, text2);
		hw.force_fan_speed(0, 0);	// automatic fan control
	}

	Base* Confirm::loop() {
		canceled = quit;
		unsigned long ms_now = millis();
		if (ms_last && ms_now - ms_last > 1000) {
			hw.beep();
			ms_last = ms_now;
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
		Base(title, 0, continue_to),
		message_on(message_on),
		message_off(message_off),
		value_getter(value_getter)
	{}

	void Test_switch::start(__attribute__((unused)) bool handle_heater) {
		canceled = false;
		old_state = value_getter();
		message = old_state ? message_on : message_off;
		test_count = SWITCH_TEST_COUNT;
		ms_last = millis();
	}

	Base* Test_switch::loop() {
		if (canceled || !test_count) {
			hw.beep();
			return continue_to;
		}
		unsigned long ms_now = millis();
		if (ms_now - ms_last > 250) {
			ms_last = ms_now;
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
		Base(title, 0, continue_to, &test_time, &test_speed)
	{}

	void Test_rotation::start(bool handle_heater) {
		test_time = ROTATION_TEST_TIME;
		test_speed = 10;
		old_seconds = 60 * ROTATION_TEST_TIME;
		fast_mode = true;
		draw = true;
		Base::start(handle_heater);
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
			get_position();
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
		Base(title, 0, continue_to, &test_time)
	{}

	void Test_fans::start(bool handle_heater) {
		test_time = FANS_TEST_TIME;
		fans_speed[0] = 0;
		fans_speed[1] = 100;
		old_fan_rpm[0] = 0;
		old_fan_rpm[1] = UINT16_MAX;
		old_seconds = 60 * FANS_TEST_TIME;
		draw1 = true;
		draw2 = true;
		hw.force_fan_speed(fans_speed[0], fans_speed[1]);
		Base::start(handle_heater);
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
					hw.force_fan_speed(fans_speed[0], fans_speed[1]);
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
			get_position();
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
			get_position();
			draw2 = false;
			return true;
		}
		return false;
	}


	// States::Test_uvled
	Test_uvled::Test_uvled(
		const char* title,
		Base* continue_to)
	:
		Base(title, STATE_OPTION_UVLED | STATE_OPTION_UVLED_TEMP, continue_to, &test_time)
	{}

	void Test_uvled::start(bool handle_heater) {
		test_time = UVLED_TEST_MAX_TIME;
		hw.force_fan_speed(30, 30);
		Base::start(handle_heater);
	}

	Base* Test_uvled::loop() {
		if (round_short(hw.uvled_temp_celsius) >= TEST_TEMP) {
			return continue_to;
		}
		uint16_t seconds = timer.getCurrentTimeInSeconds() - 1;
		if (!seconds) {
			error.new_text(pgmstr_led_failure, pgmstr_nopower_error);
			return &error;
		}
		return Base::loop();
	}


	// States::Test_heater
	Test_heater::Test_heater(
		const char* title,
		Base* continue_to,
		uint8_t* continue_after)
	:
		Base(title, STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP, continue_to, continue_after, nullptr, &temp)
	{}

	void Test_heater::start(bool handle_heater) {
		old_seconds = 0;
		temp = get_configured_temp(TEST_TEMP);
		hw.force_fan_speed(0, 0);
		draw = true;
		Base::start(handle_heater);
	}

	Base* Test_heater::loop() {
		if (round_short(get_configured_temp(hw.chamber_temp_celsius)) >= temp) {
			return continue_to;
		}
		uint16_t seconds = timer.getCurrentTimeInSeconds() - 1;
		if (!seconds) {
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
			hw.print_heater_fan(buffer, size);
			draw = false;
			return true;
		}
		return false;
	}

}
