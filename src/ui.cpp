#include "ui.h"
#include "i18n.h"
#include "defines.h"

using Ter = LiquidCrystal_Prusa::Terminator;

namespace UI {

	// UI::Base
	Base::Base(LiquidCrystal_Prusa& lcd) :
		lcd(lcd)
	{}


	// UI::Menu
	Menu::Menu(LiquidCrystal_Prusa& lcd) :
		Base(lcd)
	{}


	// UI::Value
	Value::Value(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min) :
		Base(lcd), label(label), value(value), units(units), max_value(max), min_value(min)
	{}

	void Value::show() {
		lcd.setCursor(1, 0);
		lcd.printClear_P(label, 19, Ter::none);
		lcd.print(value, 5, 2);
		lcd.print_P(units);
	}

	void Value::event_control_up() {
		if (value < max_value) {
			value++;
			show();
		}
	}

	void Value::event_control_down() {
		if (value > min_value) {
			value--;
			show();
		}
	}

	X_of_ten::X_of_ten(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value) :
		Value(lcd, label, value, pgmstr_xoften, 10)
	{}

	Minutes::Minutes(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, uint8_t max) :
		Value(lcd, label, value, pgmstr_minutes, max)
	{}

	Percent::Percent(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, uint8_t min) :
		Value(lcd, label, value, pgmstr_percent, 100, min)
	{}

	Temperature::Temperature(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, bool SI) :
		Value(lcd, label, value, SI ? pgmstr_celsius : pgmstr_fahrenheit, SI ? MAX_TARGET_TEMP_C : MAX_TARGET_TEMP_F, SI ? MIN_TARGET_TEMP_C : MIN_TARGET_TEMP_F)
	{}


	// UI::Option
	Option::Option(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, uint8_t count, const char** options) :
		Base(lcd), label(label), value(value), count(count), options(options)
	{
		if (value > count)
			value = 0;
	}

	void Option::show() {
		lcd.setCursor(1, 0);
		lcd.printClear_P(label, 19, Ter::none);
		lcd.setCursor(0, 2);
		lcd.printClear_P(pgmstr_emptystr, 20, Ter::none);
		uint8_t len = strlen_P(options[value]);
		if (value)
			len += 2;
		if (value < count - 1)
			len += 2;
		lcd.setCursor((20 - len) / 2, 2);
		if (value)
			lcd.print_P(pgmstr_lt);
		lcd.print_P(options[value]);
		if (value < count - 1)
			lcd.print_P(pgmstr_gt);
	}

	void Option::event_control_up() {
		if (value < count - 1) {
			value++;
			show();
		}
	}

	void Option::event_control_down() {
		if (value) {
			value--;
			show();
		}
	}


	// UI::State
	State::State(LiquidCrystal_Prusa& lcd) :
		Base(lcd)
	{}

}
