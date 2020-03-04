#pragma once

#include "LiquidCrystal_Prusa.h"
#include "hardware.h"

namespace UI {

	// UI::Base
	class Base {
	public:
		Base(LiquidCrystal_Prusa& lcd, const char* label);
		virtual char* get_menu_label(char* buffer, uint8_t buffer_size);
		virtual void show();
		Base* process_events(Events events);
		virtual void event_cover_opened();
		virtual void event_cover_closed();
		virtual void event_tank_inserted();
		virtual void event_tank_removed();
		virtual void event_button_short_press();
		virtual void event_button_long_press();
		virtual void event_control_up();
		virtual void event_control_down();
	protected:
		LiquidCrystal_Prusa &lcd;
		const char* label;
	};


	// UI::Menu
	class Menu : public Base {
	public:
		Menu(LiquidCrystal_Prusa& lcd, const char* label, Base** items, uint8_t items_count, bool is_root = false);
		void show();
		void event_control_up();
		void event_control_down();
	private:
		Base** items;
		uint8_t items_count;
		bool is_root;
		uint8_t menu_offset;
		uint8_t cursor_position;
		uint8_t max_items;
	};


	// UI:Bool
	class Bool : public Base {
	public:
		Bool(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
	private:
		uint8_t& value;
	};


	// UI::Value
	class Value : public Base {
	public:
		Value(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min = 1);
		void show();
		void event_control_up();
		void event_control_down();
	private:
		uint8_t& value;
		const char* units;
		uint8_t max_value;
		uint8_t min_value;
	};

	class X_of_ten : public Value {
	public:
		X_of_ten(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value);
	};

	class Minutes : public Value {
	public:
		Minutes(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, uint8_t max = 10);
	};

	class Percent : public Value {
	public:
		Percent(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, uint8_t min = 0);
	};

	class Temperature : public Value {
	public:
		Temperature(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, bool SI);
	};


	// UI::Option
	class Option : public Base {
	public:
		Option(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char** options, uint8_t options_count);
		void show();
		void event_control_up();
		void event_control_down();
	private:
		uint8_t& value;
		const char** options;
		uint8_t options_count;
	};


	// UI::State
	class State : public Base {
	public:
		State(LiquidCrystal_Prusa& lcd, const char* label);
	private:
	};

}
