#pragma once

#include "Countimer.h"
#include "hardware.h"
#include "i18n.h"
#include "config.h"
#include "simple_print.h"

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
		virtual void process_events(Events& events);
		virtual bool is_menu_available();
		virtual bool short_press_cancel();
		virtual const char* get_title();
		virtual const char* get_message();
		virtual uint16_t get_time();
		virtual bool get_info1(char* buffer, uint8_t size);
		virtual bool get_info2(char* buffer, uint8_t size);
		virtual float get_temperature();
		virtual const char* decrease_time();
		virtual const char* increase_time();
		virtual bool is_paused();
		virtual void pause_continue();
		virtual bool is_finished();
		virtual void cancel();
	protected:
		const char* title;
		uint8_t* const fans_duties;
		bool canceled;
	};


	// States::Timer_only
	class Timer_only : public Base {
	public:
		Timer_only(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		void stop();
		Base* loop();
		const char* get_title();
		uint16_t get_time();
		void set_continue_to(Base* to);
		bool is_paused();
		void pause_continue();
	protected:
		virtual void do_pause();
		virtual void do_continue();
		Base* continue_to;
	private:
		virtual const char* get_hw_pause_reason();
		uint8_t* continue_after;
		Countimer::CountType timer_type;
	};


	// States::Timer_motor
	class Timer_motor : public Timer_only {
	public:
		Timer_motor(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			uint8_t* speed,
			bool slow_mode,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		void stop();
	protected:
		uint8_t* speed;
		bool slow_mode;
	};


	// States::Timer_controls
	class Timer_controls : public Timer_motor {
	public:
		Timer_controls(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			uint8_t* speed,
			bool slow_mode,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		bool is_menu_available();
		const char* decrease_time();
		const char* increase_time();
	protected:
		void do_pause();
		void do_continue();
	};


	// States::Washing
	class Washing : public Timer_controls {
	public:
		Washing(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to);
		void process_events(Events& events);
	private:
		const char* get_hw_pause_reason();
	};


	// States::Curing
	class Curing : public Timer_controls {
	public:
		Curing(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to);
		void start();
		void stop();
		Base* loop();
		void process_events(Events& events);
		float get_temperature();
	protected:
		void do_pause();
		void do_continue();
	private:
		unsigned long led_us_last;
	};


	// States::Timer_heater
	class Timer_heater : public Timer_controls {
	public:
		Timer_heater(
			const char* title,
			uint8_t* fans_duties,
			uint8_t* after,
			Base* to,
			uint8_t* target_temp = nullptr,
			Countimer::CountType timer_type = Countimer::COUNT_DOWN);
		void start();
		void stop();
		void process_events(Events& events);
		float get_temperature();
	protected:
		void do_pause();
		void do_continue();
		uint8_t* const target_temp;
	};


	// States::Warmup
	class Warmup : public Timer_heater {
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
		void fill(const char* new_title, const char* new_message);
	private:
		const char* message;
		unsigned long beep_us_last;
	};


	// States::Test_switch
	class Test_switch : public Base {
	public:
		Test_switch(
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


	// States::Test_rotation
	class Test_rotation : public Timer_motor, public SimplePrint {
	public:
		Test_rotation(
			const char* title,
			Base* to);
		void start();
		Base* loop();
		bool get_info1(char* buffer, uint8_t size);
	private:
		uint8_t test_time;
		uint8_t test_speed;
		uint16_t old_seconds;
		bool draw;
	};


	// States::Test_fans
	class Test_fans : public Timer_only, public SimplePrint {
	public:
		Test_fans(
			const char* title,
			Base* to);
		void start();
		Base* loop();
		bool get_info1(char* buffer, uint8_t size);
		bool get_info2(char* buffer, uint8_t size);
	private:
		uint8_t test_time;
		uint8_t fans_speed[2];
		uint16_t old_fan_rpm[2];
		uint16_t old_seconds;
		bool draw1;
		bool draw2;
	};


	// States::Test_uvled
	class Test_uvled : public Timer_only {
	public:
		Test_uvled(
			const char* title,
			uint8_t* fans_duties,
			Base* to);
		void start();
		void stop();
		Base* loop();
		void process_events(Events& events);
		float get_temperature();
	protected:
		void do_pause();
		void do_continue();
	private:
		uint8_t test_time;
		float old_uvled_temp;
		unsigned long led_us_last;
	};

}
