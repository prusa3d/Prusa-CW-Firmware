#pragma once

#include "ui_items.h"

namespace UI {

	// UI:SN
	class SN : public Text {
	public:
		SN(const char* label);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
	};


	// UI::SI_switch
	class SI_switch : public Bool {
	public:
		SI_switch(const char* label, uint8_t& value, Temperature* const* to_change, uint8_t to_change_count);
		Base* in_menu_action();
	private:
		Temperature* const* to_change;
		uint8_t to_change_count;
	};


	// UI::Pause
	class Pause : public Base {
	public:
		Pause(Base* back);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
		Base* in_menu_action();
	private:
		Base* back;
	};


	// UI::Do_it
	class Do_it : public State {
	public:
		Do_it(const char* label, uint8_t& curing_machine_mode, Base* menu_short_press_running, Base* menu_short_press_finished);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
		void invoke();
	private:
		uint8_t& curing_machine_mode;
	};


	void init();
	void loop(Events& Events);
}
