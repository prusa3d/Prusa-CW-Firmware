#pragma once

#include "Countimer.h"
#include "hardware.h"
#include "i18n.h"
#include "config.h"

namespace States {

	class Base {
	public:
		Base(uint8_t* fans_duties = config.fans_menu_speed, uint8_t* target_temp = nullptr);
		virtual void invoke();
		virtual void cancel();
		virtual Base* loop();
//		virtual void add_leave_callback();
	protected:
		uint8_t* const fans_duties;
		uint8_t* const target_temp;
	};


	class Timer : public Base {
	public:
		Timer(uint8_t* fans_duties, uint8_t* after, Base* to, uint8_t* target_temp = nullptr, Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void invoke();
		void cancel();
		Base* loop();
		void set_continue_to(Base* to);
	protected:
		Base* continue_to;
	private:
		uint8_t* const continue_after;
		Countimer timer;
	};


	class Warmup : public Timer {
	public:
		Warmup(uint8_t* after, Base* to, uint8_t* target_temp);
		Base* loop();
	};


	class Confirm : public Base {
	public:
		Confirm(Base* to);
		void invoke();
		Base* loop();
	private:
		unsigned long us_last;
		Base* continue_to;
	};


	class Error : public Base {
	public:
		Error();
	};


	extern Base menu;
	extern Confirm confirm;
	extern Timer washing;
	extern Timer drying;
	extern Timer curing;
	extern Timer drying_curing;
	extern Timer resin;
	extern Warmup warmup_print;
	extern Warmup warmup_resin;

	void init();
	void loop();
	void change(Base* new_state);
}
