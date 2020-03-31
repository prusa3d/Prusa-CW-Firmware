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


	// States::Timer
	Timer::Timer(
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

	void Timer::start() {
		Base::start();
		hw.speed_configuration(get_curing_mode());
		hw.run_motor();
		timer.setCounter(0, *continue_after, 0, timer_type);
		timer.start();
	}

	void Timer::stop() {
		timer.stop();
		hw.stop_motor();
		Base::stop();
	}

	Base* Timer::loop() {
		if (hw.get_heater_error()) {
			return &heater_error;
		}
		timer.run();
		if (canceled || timer.isCounterCompleted()) {
			return continue_to;
		}
		return nullptr;
	}

	void Timer::event_tank_removed() {
		do_pause();
	}

	bool Timer::is_menu_available() {
		return true;
	}

	const char* Timer::get_title() {
		const char* pause_reason = get_hw_pause_reason();
		return timer.isStopped() ? (pause_reason ? pause_reason : pgmstr_paused) : title;
	}

	uint16_t Timer::get_time() {
		return timer.getCurrentTimeInSeconds();
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
		hw.speed_configuration(get_curing_mode());
		hw.run_motor();
	}

	bool Timer::get_curing_mode() {
		return false;
	}

	const char* Timer::get_hw_pause_reason() {
		if (!hw.is_tank_inserted()) {
			return pgmstr_insert_tank;
		}
		return nullptr;
	}

	void Timer::set_continue_to(Base* to) {
		continue_to = to;
	}


	// States::Curing
	Curing::Curing(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to)
	:
		Timer(title, fans_duties, after, to),
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

	void Curing::event_tank_removed() {
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

	bool Curing::get_curing_mode() {
		return true;
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


	// States::TimerHeater
	TimerHeater::TimerHeater(
		const char* title,
		uint8_t* fans_duties,
		uint8_t* after,
		Base* to,
		uint8_t* target_temp,
		Countimer::CountType timer_type)
	:
		Timer(title, fans_duties, after, to, timer_type),
		target_temp(target_temp)
	{}

	void TimerHeater::start() {
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

	void TimerHeater::stop() {
		hw.stop_heater();
		Timer::stop();
	}

	void TimerHeater::event_tank_removed() {
	}

	void TimerHeater::event_tank_inserted() {
		do_pause();
	}

	void TimerHeater::event_cover_opened() {
		do_pause();
	}

	float TimerHeater::get_temperature() {
		return hw.chamber_temp;
	}

	void TimerHeater::do_pause() {
		hw.stop_heater();
		Timer::do_pause();
	}

	void TimerHeater::do_continue() {
		hw.run_heater();
		Timer::do_continue();
	}

	bool TimerHeater::get_curing_mode() {
		return true;
	}

	const char* TimerHeater::get_hw_pause_reason() {
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
		TimerHeater(title, config.fans_menu_speed, after, to, target_temp, Countimer::COUNT_UP)
	{}

	Base* Warmup::loop() {
		if (!config.heat_to_target_temp || hw.chamber_temp >= *target_temp) {
			return continue_to;
		}
		return TimerHeater::loop();
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


	// States::TestSwitch
	TestSwitch::TestSwitch(
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

	void TestSwitch::start() {
		Base::start();
		old_state = value_getter();
		test_count = SWITCH_TEST_COUNT;
	}

	Base* TestSwitch::loop() {
		if (canceled || !test_count) {
			return continue_to;
		}
		return Base::loop();
	}

	const char* TestSwitch::get_message() {
		bool state = value_getter();
		if (old_state != state && test_count) {
			old_state = state;
			--test_count;
		}
		return state ? message_on : message_off;
	}

}
