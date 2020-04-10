#include "states.h"

namespace States {

	// States::Base
	Base::Base(
		const char* title,
		uint8_t* fans_duties)
	:
		title(title),
		fans_duties(fans_duties),
		canceled(false)
	{}

	void Base::start() {
		hw.set_fans(fans_duties);
		canceled = false;
	}

	void Base::stop() {
	}

	Base* Base::loop() {
		return nullptr;
	}

	void Base::process_events(__attribute__((unused)) Events& events) {
	}

	bool Base::is_menu_available() {
		return false;
	}

	bool Base::short_press_cancel() {
		return false;
	}

	const char* Base::get_title() {
		return title;
	}

	const char* Base::get_message() {
		return nullptr;
	}

	uint16_t Base::get_time() {
		return UINT16_MAX;
	}

	bool Base::get_info1(__attribute__((unused)) char* buffer, __attribute__((unused)) uint8_t size) {
		return false;
	}

	bool Base::get_info2(__attribute__((unused)) char* buffer, __attribute__((unused)) uint8_t size) {
		return false;
	}

	float Base::get_temperature() {
		return -40.0;
	}

	const char* Base::decrease_time() {
		return nullptr;
	}

	const char* Base::increase_time() {
		return nullptr;
	}

	bool Base::is_paused() {
		return false;
	}

	void Base::pause_continue() {
	}

	bool Base::is_finished() {
		return false;
	}

	void Base::cancel() {
		canceled = true;
	}


	// shared counter for all Timer states (RAM saver)
	Countimer timer;

	// States::Timer_only
	Timer_only::Timer_only(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		Countimer::CountType timer_type)
	:
		Base(title, fans_duties),
		continue_to(to),
		continue_after(after),
		timer_type(timer_type)
	{}

	void Timer_only::start() {
		Base::start();
		timer.setCounter(0, *continue_after, 0, timer_type);
		timer.start();
	}

	void Timer_only::stop() {
		timer.stop();
		Base::stop();
	}

	Base* Timer_only::loop() {
		if (hw.get_heater_error()) {
			error.fill(pgmstr_heater_error, pgmstr_please_restart);
			return &error;
		}
		timer.run();
		if (canceled || timer.isCounterCompleted()) {
			return continue_to;
		}
		return nullptr;
	}

	const char* Timer_only::get_title() {
		const char* pause_reason = get_hw_pause_reason();
		return timer.isStopped() ? (pause_reason ? pause_reason : pgmstr_paused) : title;
	}

	uint16_t Timer_only::get_time() {
		return timer.getCurrentTimeInSeconds();
	}

	void Timer_only::set_continue_to(Base* to) {
		continue_to = to;
	}

	bool Timer_only::is_paused() {
		return timer.isStopped();
	}

	void Timer_only::pause_continue() {
		if (timer.isStopped()) {
			if (!get_hw_pause_reason()) {
				do_continue();
			}
		} else {
			do_pause();
		}
	}

	void Timer_only::do_pause() {
		timer.pause();
	}

	void Timer_only::do_continue() {
		timer.start();
	}

	const char* Timer_only::get_hw_pause_reason() {
		if (hw.is_tank_inserted()) {
			return pgmstr_remove_tank;
		}
		if (!hw.is_cover_closed()) {
			return pgmstr_close_cover;
		}
		return nullptr;
	}


	// States::Timer_motor
	Timer_motor::Timer_motor(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		uint8_t* speed,
		bool slow_mode,
		Countimer::CountType timer_type)
	:
		Timer_only(title, fans_duties, after, to, timer_type),
		speed(speed),
		slow_mode(slow_mode)
	{}

	void Timer_motor::start() {
		hw.speed_configuration(*speed, slow_mode);
		hw.run_motor();
		Timer_only::start();
	}

	void Timer_motor::stop() {
		hw.stop_motor();
		Timer_only::stop();
	}


	// States::Timer_controls
	Timer_controls::Timer_controls(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		uint8_t* speed,
		bool slow_mode,
		Countimer::CountType timer_type)
	:
		Timer_motor(title, fans_duties, after, to, speed, slow_mode, timer_type)
	{}

	bool Timer_controls::is_menu_available() {
		return true;
	}

	const char* Timer_controls::decrease_time() {
		uint16_t secs = timer.getCurrentTimeInSeconds();
		if (secs < INC_DEC_TIME_STEP) {
			return pgmstr_min_symb;
		} else {
			timer.setCounterInSeconds(secs - INC_DEC_TIME_STEP);
			return pgmstr_double_lt;
		}
	}

	const char* Timer_controls::increase_time() {
		uint16_t secs = timer.getCurrentTimeInSeconds();
		if (secs > 10 * 60 - INC_DEC_TIME_STEP) {
			return pgmstr_max_symb;
		} else {
			timer.setCounterInSeconds(secs + INC_DEC_TIME_STEP);
			return pgmstr_double_gt;
		}
	}

	void Timer_controls::do_pause() {
		hw.stop_motor();
		Timer_motor::do_pause();
	}

	void Timer_controls::do_continue() {
		hw.speed_configuration(*speed, slow_mode);
		hw.run_motor();
		Timer_motor::do_continue();
	}


	// States::Washing
	Washing::Washing(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to)
	:
		Timer_controls(title, fans_duties, after, to, &config.washing_speed, false)
	{}

	void Washing::process_events(Events& events) {
		if (events.tank_removed)
			do_pause();
	}

	const char* Washing::get_hw_pause_reason() {
		if (!hw.is_tank_inserted()) {
			return pgmstr_insert_tank;
		}
		return nullptr;
	}


	// States::Curing
	Curing::Curing(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to)
	:
		Timer_controls(title, fans_duties, after, to, &config.curing_speed, true),
		led_us_last(0)
	{}

	void Curing::start() {
		Timer_controls::start();
		if (!hw.is_cover_closed() || hw.is_tank_inserted()) {
			do_pause();
		} else {
			hw.run_led();
		}
	}

	void Curing::stop() {
		hw.stop_led();
		Timer_controls::stop();
	}

	Base* Curing::loop() {
		if (hw.uvled_temp_celsius < 0.0) {
			error.fill(pgmstr_led_failure, pgmstr_led_readerror);
			return &error;
		}
		if (hw.uvled_temp_celsius > UVLED_MAX_TEMP) {
			error.fill(pgmstr_led_failure, pgmstr_led_overheat);
			return &error;
		}
		if (led_us_last && millis() - led_us_last > LED_DELAY) {
			hw.run_led();
			led_us_last = 0;
		}
		return Timer_controls::loop();
	}

	void Curing::process_events(Events& events) {
		if (events.cover_opened)
			do_pause();
	}

	float Curing::get_temperature() {
		return hw.chamber_temp;
	}

	void Curing::do_pause() {
		hw.stop_led();
		Timer_controls::do_pause();
	}

	void Curing::do_continue() {
		led_us_last = millis();
		Timer_controls::do_continue();
	}


	// States::Timer_heater
	Timer_heater::Timer_heater(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		uint8_t* target_temp,
		Countimer::CountType timer_type)
	:
		Timer_controls(title, fans_duties, after, to, &config.curing_speed, true, timer_type),
		target_temp(target_temp)
	{}

	void Timer_heater::start() {
		Timer_controls::start();
		if (target_temp) {
			hw.set_target_temp(*target_temp);
		}
		if (!hw.is_cover_closed() || hw.is_tank_inserted()) {
			do_pause();
		} else {
			hw.run_heater();
		}
	}

	void Timer_heater::stop() {
		hw.stop_heater();
		Timer_controls::stop();
	}

	void Timer_heater::process_events(Events& events) {
		if (events.cover_opened)
			do_pause();
	}

	float Timer_heater::get_temperature() {
		return hw.chamber_temp;
	}

	void Timer_heater::do_pause() {
		hw.stop_heater();
		Timer_controls::do_pause();
	}

	void Timer_heater::do_continue() {
		hw.run_heater();
		Timer_controls::do_continue();
	}


	// States::Warmup
	Warmup::Warmup(
		const char* title,
		uint8_t* after,
		Base* to,
		uint8_t* target_temp)
	:
		Timer_heater(title, config.fans_menu_speed, after, to, target_temp, Countimer::COUNT_UP)
	{}

	Base* Warmup::loop() {
		if (!config.heat_to_target_temp || hw.chamber_temp >= *target_temp) {
			return continue_to;
		}
		return Timer_heater::loop();
	}

	const char* Warmup::decrease_time() {
		return nullptr;
	}

	const char* Warmup::increase_time() {
		return nullptr;
	}


	// States::Confirm
	Confirm::Confirm(
		const char* title,
		const char* message)
	:
		Base(title),
		message(message),
		beep_us_last(0)
	{}

	void Confirm::start() {
		beep_us_last = config.finish_beep_mode;
		Base::start();
	}

	Base* Confirm::loop() {
		unsigned long us_now = millis();
		if (beep_us_last && us_now - beep_us_last > 1000) {
			hw.beep();
			if (config.finish_beep_mode == 2) {
				beep_us_last = us_now;
			} else {
				beep_us_last = 0;
			}
		}
		return Base::loop();
	}

	bool Confirm::short_press_cancel() {
		return true;
	}

	const char* Confirm::get_message() {
		return message;
	}

	bool Confirm::is_finished() {
		return canceled;
	}

	void Confirm::fill(const char* new_title, const char* new_message) {
		title = new_title;
		message = new_message;
	}


	// States::Test_switch
	Test_switch::Test_switch(
		const char* title,
		const char* message_on,
		const char* message_off,
		bool (*value_getter)(),
		Base* to)
	:
		Base(title),
		message_on(message_on),
		message_off(message_off),
		value_getter(value_getter),
		continue_to(to),
		test_count(0),
		old_state(false)
	{}

	void Test_switch::start() {
		Base::start();
		old_state = value_getter();
		test_count = SWITCH_TEST_COUNT;
	}

	Base* Test_switch::loop() {
		if (canceled || !test_count) {
			hw.beep();
			return continue_to;
		}
		return Base::loop();
	}

	const char* Test_switch::get_message() {
		bool state = value_getter();
		if (old_state != state && test_count) {
			old_state = state;
			--test_count;
		}
		return state ? message_on : message_off;
	}


	// States::Test_rotation
	Test_rotation::Test_rotation(
		const char* title,
		Base* to)
	:
		Timer_motor(title, config.fans_menu_speed, &test_time, to, nullptr, false),
		test_time(ROTATION_TEST_TIME)
	{}

	void Test_rotation::start() {
		test_speed = 10;
		speed = &test_speed;
		slow_mode = false;
		old_seconds = 60 * ROTATION_TEST_TIME;
		draw = true;
		Timer_motor::start();
	}

	Base* Test_rotation::loop() {
		uint16_t seconds = timer.getCurrentTimeInSeconds();
		if (seconds != old_seconds) {
			old_seconds = seconds;
			if (seconds && !(seconds % (60 * ROTATION_TEST_TIME / 20))) {
				if (!(--test_speed)) {
					test_speed = 10;
					slow_mode = true;
				}
				hw.speed_configuration(test_speed, slow_mode, true);
				draw = true;
			}
		}
		return Timer_motor::loop();
	}

	bool Test_rotation::get_info1(char* buffer, uint8_t size) {
		if (draw) {
			buffer[0] = slow_mode ? 'C' : 'W';
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
		Base* to)
	:
		Timer_only(title, fans_speed, &test_time, to),
		test_time(FANS_TEST_TIME)
	{}

	void Test_fans::start() {
		fans_speed[0] = 0;
		fans_speed[1] = 100;
		old_fan_rpm[0] = 0;
		old_fan_rpm[1] = UINT16_MAX;
		old_seconds = 60 * FANS_TEST_TIME;
		draw1 = true;
		draw2 = true;
		Timer_only::start();
	}

	Base* Test_fans::loop() {
		uint16_t seconds = timer.getCurrentTimeInSeconds();
		if (seconds != old_seconds) {
			old_seconds = seconds;
			draw2 = true;
			if (seconds && !((seconds - 1) % (60 * FANS_TEST_TIME / 6))) {
				if (!fans_speed[0] && hw.fan_rpm[0]) {
					error.fill(pgmstr_fan1_failure, pgmstr_spinning);
					return &error;
				}
				if (fans_speed[0] && hw.fan_rpm[0] <= old_fan_rpm[0]) {
					error.fill(pgmstr_fan1_failure, pgmstr_not_spinning);
					return &error;
				}
				if (!fans_speed[1] && hw.fan_rpm[1]) {
					error.fill(pgmstr_fan2_failure, pgmstr_spinning);
					return &error;
				}
				if (fans_speed[1] && hw.fan_rpm[1] >= old_fan_rpm[1]) {
					error.fill(pgmstr_fan2_failure, pgmstr_not_spinning);
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
		return Timer_only::loop();
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
		Base* to)
	:
		Timer_only(title, fans_duties, &test_time, to),
		test_time(UVLED_TEST_TIME),
		led_us_last(0)
	{}

	void Test_uvled::start() {
		old_uvled_temp = hw.uvled_temp_celsius;
		Timer_only::start();
		if (!hw.is_cover_closed() || hw.is_tank_inserted()) {
			do_pause();
		} else {
			hw.run_led();
		}
	}

	void Test_uvled::stop() {
		hw.stop_led();
		Timer_only::stop();
	}

	Base* Test_uvled::loop() {
		if (led_us_last && millis() - led_us_last > LED_DELAY) {
			hw.run_led();
			led_us_last = 0;
		}
		if (old_uvled_temp < 0.0 || hw.uvled_temp_celsius < 0.0) {
			error.fill(pgmstr_led_failure, pgmstr_led_readerror);
			return &error;
		}
		if (hw.uvled_temp_celsius > UVLED_MAX_TEMP) {
			error.fill(pgmstr_led_failure, pgmstr_led_overheat);
			return &error;
		}
		uint16_t seconds = timer.getCurrentTimeInSeconds() - 1;
		if (!seconds && old_uvled_temp + UVLED_TEST_GAIN > hw.uvled_temp_celsius) {
			error.fill(pgmstr_led_failure, pgmstr_led_nopower);
			return &error;
		}
		return Timer_only::loop();
	}

	void Test_uvled::process_events(Events& events) {
		if (events.cover_opened)
			do_pause();
		if (events.cover_closed && !hw.is_tank_inserted())
			do_continue();
	}

	float Test_uvled::get_temperature() {
		return hw.uvled_temp;
	}

	void Test_uvled::do_pause() {
		hw.stop_led();
		Timer_only::do_pause();
	}

	void Test_uvled::do_continue() {
		led_us_last = millis();
		Timer_only::do_continue();
	}

}
