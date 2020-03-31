#pragma once

#include "Countimer.h"
#include "hardware.h"
#include "i18n.h"
#include "config.h"

namespace States {

	// States::Base
	class Base {
	public:
		Base(
			const char* title,
			uint8_t* fans_duties = config.fans_menu_speed);
		virtual void start();
		virtual void stop();
		virtual Base* loop();
		void process_events(Events& events);
		virtual void event_cover_opened();
		virtual void event_cover_closed();
		virtual void event_tank_inserted();
		virtual void event_tank_removed();
		virtual bool is_menu_available();
		virtual bool short_press_cancel();
		virtual const char* get_title();
		virtual const char* get_message();
		virtual uint16_t get_time();
		virtual float get_temperature();
		virtual const char* decrease_time();
		virtual const char* increase_time();
		virtual bool is_paused();
		virtual void pause_continue();
		virtual bool is_finished();
		virtual void cancel();
	protected:
		const char* const title;
		uint8_t* const fans_duties;
		bool canceled;
	};


	// States::Timer
	class Timer : public Base {
	public:
		Timer(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		void stop();
		Base* loop();
		void event_tank_removed();
		bool is_menu_available();
		const char* get_title();
		uint16_t get_time();
		const char* decrease_time();
		const char* increase_time();
		bool is_paused();
		void pause_continue();
		void set_continue_to(Base* to);
	protected:
		Base* continue_to;
		virtual void do_pause();
		virtual void do_continue();
	private:
		virtual bool get_curing_mode();
		virtual const char* get_hw_pause_reason();
		uint8_t* const continue_after;
		Countimer::CountType timer_type;
	};


	// States::Curing
	class Curing : public Timer {
	public:
		Curing(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to);
		void start();
		void stop();
		Base* loop();
		void event_tank_removed();
		void event_tank_inserted();
		void event_cover_opened();
		float get_temperature();
	protected:
		void do_pause();
		void do_continue();
	private:
		bool get_curing_mode();
		const char* get_hw_pause_reason();
		unsigned long led_us_last;
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
		void stop();
		void event_tank_removed();
		void event_tank_inserted();
		void event_cover_opened();
		float get_temperature();
	protected:
		uint8_t* const target_temp;
		void do_pause();
		void do_continue();
	private:
		bool get_curing_mode();
		const char* get_hw_pause_reason();
	};


	// States::Warmup
	class Warmup : public TimerHeater {
	public:
		Warmup(
			const char* title,
			uint8_t* after,
			Base* to,
			uint8_t* target_temp);
		Base* loop();
		const char* decrease_time();
		const char* increase_time();
	};


	// States::Confirm
	class Confirm : public Base {
	public:
		Confirm(
			const char* title,
			const char* message);
		void start();
		Base* loop();
		const char* get_message();
		bool is_finished();
		bool short_press_cancel();
	private:
		const char* const message;
		unsigned long beep_us_last;
	};


	// States::TestSwitch
	class TestSwitch : public Base {
	public:
		TestSwitch(
			const char* title,
			const char* message_on,
			const char* message_off,
			bool (*value_getter)(),
			Base* to);
		void start();
		Base* loop();
		const char* get_message();
	private:
		const char* const message_on;
		const char* const message_off;
		bool (*value_getter)();
		Base* continue_to;
		uint8_t test_count;
		bool old_state;
	};

}
