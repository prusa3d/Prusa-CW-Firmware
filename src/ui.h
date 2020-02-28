#pragma once

#include "LiquidCrystal_Prusa.h"

namespace UI {

	// UI::Base
	class Base {
	public:
		Base(LiquidCrystal_Prusa& lcd);

	/*
		virtual void show();
		virtual void event_cover_opened();
		virtual void event_cover_closed();
		virtual void event_tank_inserted();
		virtual void event_tank_removed();
		virtual void event_button_short_press();
		virtual void event_button_long_press();
		virtual void event_control_up();
		virtual void event_control_down();
	*/
	protected:
		LiquidCrystal_Prusa &lcd;
	};


	// UI::Menu
	class Menu : Base {
	public:
		Menu(LiquidCrystal_Prusa& lcd);

	private:

	};


	// UI::Value
	class Value : Base {
	public:
		Value(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min = 1);

		void show();
		void event_control_up();
		void event_control_down();

	private:
		const char* label;
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
	class Option : Base {
	public:
		Option(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, uint8_t count, const char** options);

		void show();
		void event_control_up();
		void event_control_down();

	private:
		const char* label;
		uint8_t& value;
		uint8_t count;
		const char** options;
	};


	// UI::State
	class State : Base {
	public:
		State(LiquidCrystal_Prusa& lcd);

	private:

	};

}
