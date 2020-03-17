#include "states.h"
#include "defines.h"

namespace States {

	Base::Base(uint8_t* fans_duties, uint8_t* target_temp) :
		fans_duties(fans_duties), target_temp(target_temp)
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


	Timer::Timer(uint8_t* fans_duties, uint8_t* after, Base* to, uint8_t* target_temp, Countimer::CountType timer_type) :
		Base(fans_duties, target_temp), continue_to(to), continue_after(after), timer(timer_type)
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


	Warmup::Warmup(uint8_t* after, Base* to, uint8_t* target_temp) :
		Timer(config.fans_menu_speed, after, to, target_temp, Countimer::COUNT_UP)
	{}

	Base* Warmup::loop() {
		if (!config.heat_to_target_temp || hw.chamber_temp >= *target_temp) {
			return continue_to;
		}
		return Timer::loop();
	}


	Confirm::Confirm(Base* to) :
		Base(), us_last(0), continue_to(to)
	{}

	void Confirm::invoke() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		us_last = 0;
		Base::invoke();
	}

	Base* Confirm::loop() {
		unsigned long us_now = millis();
		if (config.finish_beep_mode && us_now - us_last > 1000) {
			hw.beep();
			us_last = us_now;
		}
		if (config.finish_beep_mode == 2) {
			return nullptr;
		}
		return continue_to;
	}


	Error::Error() :
		Base()
	{}


	/*** states definitions ***/
	Base menu;
	Confirm confirm(&menu);
	Timer washing(config.fans_washing_speed, &config.washing_run_time, &confirm);
	// FIXME - would be better to set PI regulator and manage heater for drying/curing?
	Timer drying(config.fans_drying_speed, &config.drying_run_time, &confirm);
	Timer curing(config.fans_curing_speed, &config.curing_run_time, &confirm);
	Timer drying_curing(config.fans_drying_speed, &config.drying_run_time, &curing);
	Timer resin(config.fans_menu_speed, &config.resin_preheat_run_time, &confirm, &config.resin_target_temp);
	uint8_t max_warmup_run_time = MAX_WARMUP_RUN_TIME;
	Warmup warmup_print(&max_warmup_run_time, nullptr, &config.target_temp);
	Warmup warmup_resin(&max_warmup_run_time, &resin, &config.resin_target_temp);


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
