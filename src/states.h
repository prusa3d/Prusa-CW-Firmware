#pragma once

#include "Countimer.h"
#include "hardware.h"
#include "i18n.h"
#include "config.h"

namespace States {

	class Base {
	public:
		Base(const char* title, uint8_t* fans_duties = config.fans_menu_speed, uint8_t* target_temp = nullptr);
		virtual void start();
		virtual void stop();
		virtual Base* loop();
		virtual const char* get_title();
		virtual const char* get_message();
		virtual uint16_t get_time();
		virtual float get_temperature();
		virtual const char* decrease_time();
		virtual const char* increase_time();
		virtual bool is_running();
		virtual void pause_continue();
	protected:
		const char* const title;
		uint8_t* const fans_duties;
		uint8_t* const target_temp;
	};


	class Timer : public Base {
	public:
		Timer(const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			bool return_temperature = false,
			uint8_t* target_temp = nullptr,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		void stop();
		Base* loop();
		const char* get_title();
		uint16_t get_time();
		float get_temperature();
		const char* decrease_time();
		const char* increase_time();
		bool is_running();
		void pause_continue();
		void set_continue_to(Base* to);
	protected:
		Base* continue_to;
	private:
		uint8_t* const continue_after;
		bool return_temperature;
		Countimer timer;
	};


	class Warmup : public Timer {
	public:
		Warmup(const char* title, uint8_t* after, Base* to, uint8_t* target_temp);
		Base* loop();
		const char* decrease_time();
		const char* increase_time();
	};


	class Confirm : public Base {
	public:
		Confirm(const char* title);
		void start();
		Base* loop();
		const char* get_message();
	private:
		unsigned long us_last;
		uint8_t beep;
	};

	extern Base* active_state;

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
	void loop(Events& events);
	void change(Base* new_state);
}
