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
			uint8_t* motor_speed = nullptr,
			uint8_t* target_temp = nullptr);

		virtual void start();
		virtual Base* loop();
		virtual bool get_info1(char* buffer, uint8_t size);
		virtual bool get_info2(char* buffer, uint8_t size);

		void do_pause();
		void do_continue();
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
	protected:
		Base* continue_to;
		const char* message;
		uint8_t* target_temp;
		unsigned long ms_last;
		bool canceled;
	private:
		const char* get_hw_pause_reason();
		const char* title;
		uint8_t* const continue_after;
		uint8_t* const motor_speed;
		uint8_t const options;
	};


	// States::Warmup
	class Warmup : public Base {
	public:
		Warmup(
			const char* title,
			Base* continue_to,
			uint8_t* continue_after,
			uint8_t* motor_speed,
			uint8_t* target_temp);
		Base* loop();
	};


	// States::Cooldown
	class Cooldown : public Base {
	public:
		Cooldown(Base* continue_to, uint8_t* continue_after);
		void start();
	};


	// States::Confirm
	class Confirm : public Base {
	public:
		Confirm(bool force_wait);
		void start();
		Base* loop();
	private:
		bool force_wait;
		bool quit;
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
		void start();
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
		void start();
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
	class Test_uvled : public Base {
	public:
		Test_uvled(
			const char* title,
			Base* to);
		void start();
		Base* loop();
	private:
		uint8_t test_time;
		float old_uvled_temp;
	};


	// States::Test_heater
	class Test_heater : public Base {
	public:
		Test_heater(
			const char* title,
			Base* to);
		void start();
		Base* loop();
		bool get_info2(char* buffer, uint8_t size);
	private:
		uint8_t test_time;
		uint8_t new_chamb_temp;
		float old_chamb_temp;
		uint16_t old_seconds;
		bool draw;
	};

}
