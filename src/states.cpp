#include "states.h"
#include "defines.h"

namespace States {

	Base::Base(const char* title, uint8_t* fans_duties, uint8_t* target_temp) :
		title(title), fans_duties(fans_duties), target_temp(target_temp)
	{}

	void Base::invoke() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		hw.set_fans(fans_duties, target_temp);
	}

	void Base::cancel() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	Base* Base::loop() {
		return nullptr;
	}

	const char* Base::get_title() {
		return title;
	}

	const char* Base::get_message() {
		return nullptr;
	}


	Timer::Timer(const char* title, uint8_t* fans_duties, uint8_t* after, Base* to, uint8_t* target_temp, Countimer::CountType timer_type) :
		Base(title, fans_duties, target_temp), continue_to(to), continue_after(after), timer(timer_type)
	{}

	void Timer::invoke() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		timer.setCounter(0, *continue_after, 0);
		timer.start();
		Base::invoke();
	}

	void Timer::cancel() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		timer.stop();
		Base::cancel();
	}

	Base* Timer::loop() {
		timer.run();
		if (timer.isCounterCompleted()) {
			return continue_to;
		}
		static uint8_t last = -1;
		uint8_t actual = timer.getCurrentSeconds();
		if (last != actual) {
			USB_PRINTLN(actual);
			last = actual;
		}
		return nullptr;
	}

	void Timer::set_continue_to(Base* to) {
		continue_to = to;
	}


	Warmup::Warmup(const char* title, uint8_t* after, Base* to, uint8_t* target_temp) :
		Timer(title, config.fans_menu_speed, after, to, target_temp, Countimer::COUNT_UP)
	{}

	Base* Warmup::loop() {
		if (!config.heat_to_target_temp || hw.chamber_temp >= *target_temp) {
			return continue_to;
		}
		return Timer::loop();
	}


	Confirm::Confirm(const char* title) :
		Base(title), us_last(0), beep(0)
	{}

	void Confirm::invoke() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		us_last = 0;
		beep = config.finish_beep_mode;
		Base::invoke();
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
	Timer drying(pgmstr_drying, config.fans_drying_speed, &config.drying_run_time, &confirm);
	Timer curing(pgmstr_curing, config.fans_curing_speed, &config.curing_run_time, &confirm);
	Timer drying_curing(pgmstr_drying, config.fans_drying_speed, &config.drying_run_time, &curing);
	Timer resin(pgmstr_heating, config.fans_menu_speed, &config.resin_preheat_run_time, &confirm, &config.resin_target_temp);
	uint8_t max_warmup_run_time = MAX_WARMUP_RUN_TIME;
	Warmup warmup_print(pgmstr_warmup, &max_warmup_run_time, nullptr, &config.target_temp);
	Warmup warmup_resin(pgmstr_warmup, &max_warmup_run_time, &resin, &config.resin_target_temp);


	// states data
	Base* active_state = &menu;

	void init() {
		active_state->invoke();
	}

	void loop() {
		Base* new_state = active_state->loop();
		if (new_state) {
			change(new_state);
		}
	}

	void change(Base* new_state) {
		active_state->cancel();
		active_state = new_state;
		active_state->invoke();
	}
}
