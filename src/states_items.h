#pragma once

#include "Countimer.h"
#include "device.h"
#include "i18n.h"
#include "config.h"
#include "simple_print.h"

#define STATE_OPTION_TIMER_UP		1
#define STATE_OPTION_UVLED			2
#define STATE_OPTION_HEATER			4
#define STATE_OPTION_CONTROLS		8
#define STATE_OPTION_WASHING		16
#define STATE_OPTION_SHORT_CANCEL	32
#define STATE_OPTION_CHAMB_TEMP		64
#define STATE_OPTION_UVLED_TEMP		128

namespace States {

	// States::Base
	class Base {

	public:
		Base(
			const char* title = pgmstr_emptystr,
			uint8_t options = 0,
			Base* continue_to = nullptr,
			uint8_t* continue_after = nullptr,
			uint8_t max_runtime = 10,
			uint8_t* motor_speed = nullptr,
			uint8_t* target_temp = nullptr);

		virtual void start(bool handle_heater = true);
		virtual Base* loop();
		virtual bool get_info1(char* buffer, uint8_t size);
		virtual bool get_info2(char* buffer, uint8_t size);

		void do_pause(bool handle_heater = true);
		void do_continue(bool handle_heater = true);
		void pause_continue();
		void cancel();
		void process_events(uint8_t events);
		bool short_press_cancel();
		const char* get_title();
		const char* get_message();
		uint16_t get_time();
		float get_temperature();
		const char* decrease_time();
		const char* increase_time();
		bool is_paused();
		bool is_finished();
		void set_continue_to(Base* to);
		void new_text(const char* new_title, const char* new_message);
		uint8_t const options;
	protected:
		Base* continue_to;
		const char* message;
		uint8_t* target_temp;
		uint8_t* const motor_speed;
		uint8_t* const continue_after;
		unsigned long ms_last;
		bool canceled;
		bool motor_direction;
	private:
		const char* get_hw_pause_reason();
		uint16_t max_runtime;
		const char* title;
	};


	// States::Direction_change
	class Direction_change : public Base {
	public:
		Direction_change(
			const char* title,
			uint8_t options,
			uint8_t* direction_cycles,
			Base* continue_to = nullptr,
			uint8_t* continue_after = nullptr,
			uint8_t max_runtime = 10,
			uint8_t* motor_speed = nullptr,
			uint8_t* target_temp = nullptr);
		void start(bool handle_heater = true);
		Base* loop();
	private:
		uint8_t* const direction_cycles;
		uint16_t direction_change_time;
		uint16_t old_seconds;
		uint16_t stop_seconds;
	};


	// States::Warmup
	class Warmup : public Base {
	public:
		Warmup(
			const char* title,
			Base* continue_to,
			uint8_t* target_temp);
		Base* loop();
	private:
		uint8_t continue_after;
	};


	// States::Cooldown
	class Cooldown : public Base {
	public:
		Cooldown(Base* continue_to);
		void start(bool handle_heater = true);
	private:
		uint8_t cooldown_time;
	};


	// States::Confirm
	class Confirm : public Base {
	public:
		Confirm(bool force_wait);
		void start(bool handle_heater = true);
		Base* loop();
	private:
		bool force_wait;
		bool quit;
	};


	// States::Reset
	class Reset : public Base {
	public:
		Reset();
		void start(bool handle_heater = true);
	};


	// States::Test_switch
	class Test_switch : public Base {
	public:
		Test_switch(
			const char* title,
			Base* continue_to,
			const char* message_on,
			const char* message_off,
			bool (*value_getter)());
		void start(bool handle_heater = true);
		Base* loop();
	private:
		const char* const message_on;
		const char* const message_off;
		bool (*value_getter)();
		uint8_t test_count;
		bool old_state;
	};


	// States::Test_rotation
	class Test_rotation : public Base, public SimplePrint {
	public:
		Test_rotation(
			const char* title,
			Base* continue_to);
		void start(bool handle_heater = true);
		Base* loop();
		bool get_info1(char* buffer, uint8_t size);
	private:
		uint8_t test_time;
		uint8_t test_speed;
		uint16_t old_seconds;
		bool fast_mode;
		bool draw;
	};


	// States::Test_fans
	class Test_fans : public Base, public SimplePrint {
	public:
		Test_fans(
			const char* title,
			Base* continue_to);
		void start(bool handle_heater = true);
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
	class Test_uvled : public Base {
	public:
		Test_uvled(
			const char* title,
			Base* continue_to);
		void start(bool handle_heater = true);
		Base* loop();
	private:
		uint8_t test_time;
	};


	// States::Test_heater
	class Test_heater : public Base {
	public:
		Test_heater(
			const char* title,
			Base* continue_to);
		void start(bool handle_heater = true);
		Base* loop();
		bool get_info2(char* buffer, uint8_t size);
	private:
		uint8_t test_time;
		uint8_t temp;
		uint16_t old_seconds;
		bool draw;
	};

}
