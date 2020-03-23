#pragma once

#include "Countimer.h"
#include "hardware.h"
#include "i18n.h"
#include "config.h"

namespace States {

	// States::Base
	class Base {
	public:
		Base(const char* title, uint8_t* fans_duties = config.fans_menu_speed);
		virtual void start();
		virtual void stop();
		virtual Base* loop();
		void process_events(Events& events);
		virtual void event_cover_opened();
		virtual void event_cover_closed();
		virtual void event_tank_inserted();
		virtual void event_tank_removed();
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
	};


	// States::Timer
	class Timer : public Base {
	public:
		Timer(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			bool curing_mode = false,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		void stop();
		Base* loop();
		void event_tank_removed();
		const char* get_title();
		uint16_t get_time();
		const char* decrease_time();
		const char* increase_time();
		bool is_running();
		void pause_continue();
		void set_continue_to(Base* to);
	protected:
		Base* continue_to;
	private:
		void do_pause();
		void do_continue();
		uint8_t* const continue_after;
		bool curing_mode;
		Countimer timer;
	};


	// States::TimerHeater
	class TimerHeater : public Timer {
	public:
		TimerHeater(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			uint8_t* target_temp = nullptr,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		float get_temperature();
	protected:
		uint8_t* const target_temp;
	};


	// States::Warmup
	class Warmup : public TimerHeater {
	public:
		Warmup(const char* title, uint8_t* after, Base* to, uint8_t* target_temp);
		Base* loop();
		const char* decrease_time();
		const char* increase_time();
	};


	// States::Confirm
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


	/*** states definitions ***/
	extern Base* active_state;

	extern Base menu;
	extern Confirm confirm;
	extern Timer washing;
	extern TimerHeater drying;
	extern TimerHeater curing;
	extern TimerHeater drying_curing;
	extern TimerHeater resin;
	extern Warmup warmup_print;
	extern Warmup warmup_resin;

	void init();
	void loop(Events& events);
	void change(Base* new_state);
}
