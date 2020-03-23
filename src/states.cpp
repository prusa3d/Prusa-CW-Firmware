#include "states.h"
#include "defines.h"

namespace States {

	// States::Base
	Base::Base(const char* title, uint8_t* fans_duties) :
		title(title), fans_duties(fans_duties)
	{}

	void Base::start() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		hw.set_fans(fans_duties);
	}

	void Base::stop() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
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
		bool curing_mode,
		Countimer::CountType timer_type)
	:
		Base(title, fans_duties),
		continue_to(to),
		continue_after(after),
		curing_mode(curing_mode),
		timer(timer_type)
	{}

	void Timer::start() {
		Base::start();
		USB_PRINTLN(__PRETTY_FUNCTION__);
		hw.speed_configuration(curing_mode);
		hw.run_motor();
		timer.setCounter(0, *continue_after, 0);
		timer.start();
	}

	void Timer::stop() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		timer.stop();
		hw.stop_motor();
		Base::stop();
	}

	Base* Timer::loop() {
		timer.run();
		if (timer.isCounterCompleted()) {
			return continue_to;
		}
		return nullptr;
	}

	void Timer::event_tank_removed() {
		do_pause();
	}

	const char* Timer::get_title() {
		// TODO better!
		return timer.isStopped() ? (hw.is_tank_inserted() ? pgmstr_paused : pgmstr_ipa_tank_removed) : title;
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
			// TODO better!
			if (hw.is_tank_inserted()) {
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
		hw.speed_configuration(curing_mode);
		hw.run_motor();
	}

	void Timer::set_continue_to(Base* to) {
		continue_to = to;
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
		Timer(title, fans_duties, after, to, true, timer_type), target_temp(target_temp)
	{}

	void TimerHeater::start() {
		Timer::start();
		USB_PRINTLN(__PRETTY_FUNCTION__);
		hw.set_target_temp(*target_temp);
	}

	float TimerHeater::get_temperature() {
		return hw.chamber_temp;
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
	Confirm::Confirm(const char* title) :
		Base(title), us_last(0), beep(0)
	{}

	void Confirm::start() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		us_last = 0;
		beep = config.finish_beep_mode;
		Base::start();
	}

	Base* Confirm::loop() {
		unsigned long us_now = millis();
		if (beep && us_now - us_last > 1000) {
			hw.beep();
			us_last = us_now;
			if (beep != 2) {
				beep = 0;
			}
		}
		return nullptr;
	}

	const char* Confirm::get_message() {
		return pgmstr_press2continue;
	}


	/*** states definitions ***/
	Base menu(pgmstr_emptystr);
	Confirm confirm(pgmstr_finished);
	Timer washing(pgmstr_washing, config.fans_washing_speed, &config.washing_run_time, &confirm);
	// FIXME - would be better to set PI regulator and manage heater for drying/curing?
	TimerHeater drying(pgmstr_drying, config.fans_drying_speed, &config.drying_run_time, &confirm);
	TimerHeater curing(pgmstr_curing, config.fans_curing_speed, &config.curing_run_time, &confirm);
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
		active_state->process_events(events);
		Base* new_state = active_state->loop();
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
