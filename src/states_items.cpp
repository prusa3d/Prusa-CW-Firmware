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

	void Base::process_events(Events& events) {
		if (events.cover_opened)
			event_cover_opened();
		if (events.cover_closed)
			event_cover_closed();
		if (events.tank_inserted)
			event_tank_inserted();
		if (events.tank_removed)
			event_tank_removed();
	}

	void Base::event_cover_opened() {
	}

	void Base::event_cover_closed() {
	}

	void Base::event_tank_inserted() {
	}

	void Base::event_tank_removed() {
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

	char* Base::get_text() {
		return nullptr;
	}

	float Base::get_temperature() {
		return -1.0;
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


	// States::Timer_no_controls
	Timer_no_controls::Timer_no_controls(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		uint8_t* speed,
		bool slow_mode,
		Countimer::CountType timer_type)
	:
		Base(title, fans_duties),
		continue_to(to),
		speed(speed),
		slow_mode(slow_mode),
		continue_after(after),
		timer_type(timer_type)
	{}

	void Timer_no_controls::start() {
		Base::start();
		hw.speed_configuration(*speed, slow_mode);
		hw.run_motor();
		timer.setCounter(0, *continue_after, 0, timer_type);
		timer.start();
	}

	void Timer_no_controls::stop() {
		timer.stop();
		hw.stop_motor();
		Base::stop();
	}

	Base* Timer_no_controls::loop() {
		if (hw.get_heater_error()) {
			return &heater_error;
		}
		timer.run();
		if (canceled || timer.isCounterCompleted()) {
			return continue_to;
		}
		return nullptr;
	}

	uint16_t Timer_no_controls::get_time() {
		return timer.getCurrentTimeInSeconds();
	}

	void Timer_no_controls::set_continue_to(Base* to) {
		continue_to = to;
	}


	// States::Timer
	Timer::Timer(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		uint8_t* speed,
		bool slow_mode,
		Countimer::CountType timer_type)
	:
		Timer_no_controls(title, fans_duties, after, to, speed, slow_mode, timer_type)
	{}

	bool Timer::is_menu_available() {
		return true;
	}

	const char* Timer::get_title() {
		const char* pause_reason = get_hw_pause_reason();
		return timer.isStopped() ? (pause_reason ? pause_reason : pgmstr_paused) : title;
	}

	const char* Timer::decrease_time() {
		uint16_t secs = timer.getCurrentTimeInSeconds();
		if (secs < INC_DEC_TIME_STEP) {
			return pgmstr_min_symb;
		} else {
			timer.setCounterInSeconds(secs - INC_DEC_TIME_STEP);
			return pgmstr_double_lt;
		}
	}

	const char* Timer::increase_time() {
		uint16_t secs = timer.getCurrentTimeInSeconds();
		if (secs > 10 * 60 - INC_DEC_TIME_STEP) {
			return pgmstr_max_symb;
		} else {
			timer.setCounterInSeconds(secs + INC_DEC_TIME_STEP);
			return pgmstr_double_gt;
		}
	}

	bool Timer::is_paused() {
		return timer.isStopped();
	}

	void Timer::pause_continue() {
		if (timer.isStopped()) {
			if (!get_hw_pause_reason()) {
				do_continue();
			}
		} else {
			do_pause();
		}
	}

	void Timer::do_pause() {
		timer.pause();
		hw.stop_motor();
	}

	void Timer::do_continue() {
		timer.start();
		hw.speed_configuration(*speed, slow_mode);
		hw.run_motor();
	}


	// States::Washing
	Washing::Washing(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to)
	:
		Timer(title, fans_duties, after, to, &config.washing_speed, false)
	{}

	void Washing::event_tank_removed() {
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
		Timer(title, fans_duties, after, to, &config.curing_speed, true),
		led_us_last(0)
	{}

	void Curing::start() {
		Timer::start();
		if (!hw.is_cover_closed()) {
			do_pause();
		} else {
			hw.run_led();
		}
	}

	void Curing::stop() {
		hw.stop_led();
		Timer::stop();
	}

	Base* Curing::loop() {
		if (led_us_last && millis() - led_us_last > LED_DELAY) {
			hw.run_led();
			led_us_last = 0;
		}
		return Timer::loop();
	}

	void Curing::event_tank_inserted() {
		do_pause();
	}

	void Curing::event_cover_opened() {
		do_pause();
	}

	float Curing::get_temperature() {
		return hw.chamber_temp;
	}

	void Curing::do_pause() {
		hw.stop_led();
		Timer::do_pause();
	}

	void Curing::do_continue() {
		led_us_last = millis();
		Timer::do_continue();
	}

	const char* Curing::get_hw_pause_reason() {
		if (hw.is_tank_inserted()) {
			return pgmstr_remove_tank;
		}
		if (!hw.is_cover_closed()) {
			return pgmstr_close_cover;
		}
		return nullptr;
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
		Timer(title, fans_duties, after, to, &config.curing_speed, true, timer_type),
		target_temp(target_temp)
	{}

	void Timer_heater::start() {
		Timer::start();
		if (target_temp) {
			hw.set_target_temp(*target_temp);
		}
		if (!hw.is_cover_closed()) {
			do_pause();
		} else {
			hw.run_heater();
		}
	}

	void Timer_heater::stop() {
		hw.stop_heater();
		Timer::stop();
	}

	void Timer_heater::event_tank_inserted() {
		do_pause();
	}

	void Timer_heater::event_cover_opened() {
		do_pause();
	}

	float Timer_heater::get_temperature() {
		return hw.chamber_temp;
	}

	void Timer_heater::do_pause() {
		hw.stop_heater();
		Timer::do_pause();
	}

	void Timer_heater::do_continue() {
		hw.run_heater();
		Timer::do_continue();
	}

	const char* Timer_heater::get_hw_pause_reason() {
		if (hw.is_tank_inserted()) {
			return pgmstr_remove_tank;
		}
		if (!hw.is_cover_closed()) {
			return pgmstr_close_cover;
		}
		return nullptr;
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
		Timer_no_controls(title, config.fans_menu_speed, nullptr, to, nullptr, false),
		test_time(ROTATION_TEST_TIME)
	{}

	void Test_rotation::start() {
		test_speed = 10;
		speed = &test_speed;
		slow_mode = false;
		continue_after = &test_time;
		old_seconds = 60 * ROTATION_TEST_TIME;
		Timer_no_controls::start();
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
			}
		}
		return Timer_no_controls::loop();
	}

	char* Test_rotation::get_text() {
		memset(buffer, ' ', sizeof(buffer));
		buffer[sizeof(buffer)-1] = char(0);	// end of text
		buffer[0] = slow_mode ? 'C' : 'W';
		position = 1;
		print(test_speed, 10, 0);
		USB_PRINTLN(buffer);
		return buffer;
	}

	void Test_rotation::write(uint8_t c) {
		if (position < sizeof(buffer)) {
			buffer[position] = c;
			++position;
		}
	}

}
