#pragma once

#include "LiquidCrystal_Prusa.h"
#include "hardware.h"
#include "i18n.h"

namespace UI {

	// UI::Base
	class Base {
	public:
		Base(LiquidCrystal_Prusa& lcd, const char* label, uint8_t last_char = RIGHT_CHAR, bool menu_action = false);
		virtual char* get_menu_label(char* buffer, uint8_t buffer_size);
		virtual void show();
		Base* process_events(Events events);
		virtual void event_cover_opened();
		virtual void event_cover_closed();
		virtual void event_tank_inserted();
		virtual void event_tank_removed();
		virtual Base* event_button_short_press();
		virtual Base* event_button_long_press();
		virtual void event_control_up();
		virtual void event_control_down();
		virtual bool in_menu_action();
		void set_long_press_ui_item(Base *ui_item);
	protected:
		LiquidCrystal_Prusa &lcd;
		const char* label;
		uint8_t last_char;
		bool menu_action;
		Base* long_press_ui_item;
	};


	// UI:SN
	class SN : public Base {
	public:
		SN(LiquidCrystal_Prusa& lcd, const char* label);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
	};


	// UI::Menu
	class Menu : public Base {
	public:
		Menu(LiquidCrystal_Prusa& lcd, const char* label, Base* const* items, uint8_t items_count);
		void show();
		Base* event_button_short_press();
		void event_control_up();
		void event_control_down();
	private:
		Base* const* items;
		uint8_t items_count;
		uint8_t menu_offset;
		uint8_t cursor_position;
		uint8_t max_items;
	};


	// UI::Value
	class Value : public Base {
	public:
		Value(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min = 1);
		void show();
		Base* event_button_short_press();
		void event_control_up();
		void event_control_down();
	private:
		const char* units;
	protected:
		uint8_t& value;
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
		void units_change(bool SI);
	};


	// UI:Bool
	class Bool : public Base {
	public:
		Bool(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char* true_text = pgmstr_on, const char* false_text = pgmstr_off);
		char* get_menu_label(char* buffer, uint8_t buffer_size);
		bool in_menu_action();
	private:
		const char* true_text;
		const char* false_text;
	protected:
		uint8_t& value;
	};

	class SI_switch : public Bool {
	public:
		SI_switch(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, Temperature* const* to_change, uint8_t to_change_count);
		bool in_menu_action();
	private:
		Temperature* const* to_change;
		uint8_t to_change_count;
	};


	// UI::Option
	class Option : public Base {
	public:
		Option(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char** options, uint8_t options_count);
		void show();
		Base* event_button_short_press();
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
