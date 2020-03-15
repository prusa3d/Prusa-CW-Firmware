#pragma once

#include "ui_items.h"

namespace UI {

	// UI:SN
	class SN : public Base {
	public:
		SN(const char* label);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
	};


	// UI::SI_switch
	class SI_switch : public Bool {
	public:
		SI_switch(const char* label, uint8_t& value, Temperature* const* to_change, uint8_t to_change_count);
		bool in_menu_action();
	private:
		Temperature* const* to_change;
		uint8_t to_change_count;
	};


	// UI::Do_it
	class Do_it : public State {
	public:
		Do_it(const char* label, uint8_t& curing_machine_mode, States::Base* long_press_state);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
		void show();
	private:
		uint8_t& curing_machine_mode;
	};


	void init();
	void loop();
}
