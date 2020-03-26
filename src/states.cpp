#include "states.h"
#include "defines.h"

namespace States {

	// States::Base
	Base::Base(const char* title, uint8_t* fans_duties) :
		title(title), fans_duties(fans_duties)
	{}

	void Base::start() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		hw.set_fans(fans_duties);
	}

	void Base::stop() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	Base* Base::loop() {
		return nullptr;
	}

	Base* Base::process_events(Events& events) {
		if (events.cover_opened)
			event_cover_opened();
		if (events.cover_closed)
			event_cover_closed();
		if (events.tank_inserted)
			event_tank_inserted();
		if (events.tank_removed)
			event_tank_removed();
		if (events.button_long_press)
			return event_button_long_press();
		return nullptr;
	}

	void Base::event_cover_opened() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	void Base::event_cover_closed() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	void Base::event_tank_inserted() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	void Base::event_tank_removed() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	Base* Base::event_button_long_press() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		return nullptr;
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

	bool Base::is_running() {
		return true;
	}

	void Base::pause_continue() {
	}


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
		timer(timer_type)
	{}

	void Timer::start() {
		Base::start();
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		hw.speed_configuration(get_curing_mode());
		hw.run_motor();
		timer.setCounter(0, *continue_after, 0);
		timer.start();
	}

	void Timer::stop() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		timer.stop();
		hw.stop_motor();
		Base::stop();
	}

	Base* Timer::loop() {
		if (hw.get_heater_error()) {
			return &heater_error;
		}
		timer.run();
		if (timer.isCounterCompleted()) {
			return continue_to;
		}
		return nullptr;
	}

	void Timer::event_tank_removed() {
		do_pause();
	}

	Base* Timer::event_button_long_press() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		return continue_to;
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

	bool Timer::is_running() {
		return !timer.isStopped();
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
		Timer(title, fans_duties, after, to), led_us_last(0)
	{}

	void Curing::start() {
		Timer::start();
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (!hw.is_cover_closed()) {
			do_pause();
		} else {
			hw.run_led();
		}
	}

	void Curing::stop() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
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
		Timer(title, fans_duties, after, to, timer_type), target_temp(target_temp)
	{}

	void TimerHeater::start() {
		Timer::start();
//		USB_PRINTLN(__PRETTY_FUNCTION__);
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
//		USB_PRINTLN(__PRETTY_FUNCTION__);
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
	Warmup::Warmup(const char* title, uint8_t* after, Base* to, uint8_t* target_temp) :
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
	Confirm::Confirm(const char* title, const char* message) :
		Base(title), message(message), beep_us_last(0)
	{}

	void Confirm::start() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
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
		return nullptr;
	}

	const char* Confirm::get_message() {
		return message;
	}


	/*** states definitions ***/
	Base menu(pgmstr_emptystr);
	Confirm confirm(pgmstr_finished, pgmstr_press2continue);
	Confirm heater_error(pgmstr_heater_error, pgmstr_please_restart);
	Timer washing(pgmstr_washing, config.fans_washing_speed, &config.washing_run_time, &confirm);
	// FIXME - would be better to set PI regulator and manage heater for drying/curing?
	TimerHeater drying(pgmstr_drying, config.fans_drying_speed, &config.drying_run_time, &confirm);
	Curing curing(pgmstr_curing, config.fans_curing_speed, &config.curing_run_time, &confirm);
	TimerHeater drying_curing(pgmstr_drying, config.fans_drying_speed, &config.drying_run_time, &curing);
	TimerHeater resin(pgmstr_heating, config.fans_menu_speed, &config.resin_preheat_run_time, &confirm, &config.resin_target_temp);
	uint8_t max_warmup_run_time = MAX_WARMUP_RUN_TIME;
	Warmup warmup_print(pgmstr_warmup, &max_warmup_run_time, nullptr, &config.target_temp);
	Warmup warmup_resin(pgmstr_warmup, &max_warmup_run_time, &resin, &config.resin_target_temp);


	// states data
	Base* active_state = &menu;

	void init() {
		active_state->start();
	}

	void loop(Events& events) {
		Base* new_state = active_state->process_events(events);
		if (!new_state) {
			new_state = active_state->loop();
		}
		if (new_state) {
			change(new_state);
		}
	}

	void change(Base* new_state) {
		active_state->stop();
		active_state = new_state;
		active_state->start();
	}
}
