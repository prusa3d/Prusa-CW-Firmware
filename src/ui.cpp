#include "ui.h"
#include "i18n.h"
#include "defines.h"

using Ter = LiquidCrystal_Prusa::Terminator;

namespace UI {

	// UI::Base
	Base::Base(LiquidCrystal_Prusa& lcd, const char* label) :
		lcd(lcd), label(label)
	{}

	char* Base::get_menu_label(char* buffer, uint8_t buffer_size) {
		USB_TRACE("Base::get_menu_label()\r\n");
		buffer[--buffer_size] = char(0);	// end of text
		buffer[--buffer_size] = char(RIGHT_CHAR);
		memset(buffer, ' ', buffer_size);
		const char* from = label;
		uint8_t c = pgm_read_byte(from);
		while (buffer_size && c) {
			*buffer = c;
			++buffer;
			--buffer_size;
			c = pgm_read_byte(++from);
		}
	return buffer;
	}

	void Base::show() {
		USB_TRACE("Base::show()\r\n");
		// do nothing
	}

	Base* Base::process_events(Events events) {
		if (events.cover_opened)
			event_cover_opened();
		if (events.cover_closed)
			event_cover_closed();
		if (events.tank_inserted)
			event_tank_inserted();
		if (events.tank_removed)
			event_tank_removed();
		if (events.control_up)
			event_control_up();
		if (events.control_down)
			event_control_down();
		if (events.button_short_press)
			event_button_short_press();
		if (events.button_long_press)
			event_button_long_press();
		return nullptr;		// TODO for change menu page
	}

	void Base::event_cover_opened() {
		USB_TRACE("Base::event_cover_opened()\r\n");
		// do nothing
	}

	void Base::event_cover_closed() {
		USB_TRACE("Base::event_cover_closed()\r\n");
		// do nothing
	}

	void Base::event_tank_inserted() {
		USB_TRACE("Base::event_tank_inserted()\r\n");
		// do nothing
	}

	void Base::event_tank_removed() {
		USB_TRACE("Base::event_tank_removed()\r\n");
		// do nothing
	}

	void Base::event_button_short_press() {
		USB_TRACE("Base::event_button_short_press()\r\n");
		// do nothing
	}

	void Base::event_button_long_press() {
		USB_TRACE("Base::event_button_long_press()\r\n");
		// do nothing
	}

	void Base::event_control_up() {
		USB_TRACE("Base::event_control_up()\r\n");
		// do nothing
	}

	void Base::event_control_down() {
		USB_TRACE("Base::event_control_down()\r\n");
		// do nothing
	}


	// UI::Menu
	Menu::Menu(LiquidCrystal_Prusa& lcd, const char* label, Base** items, uint8_t items_count, bool is_root) :
		Base(lcd, label), items(items), items_count(items_count), is_root(is_root), menu_offset(0), cursor_position(0)
	{
		max_items = items_count < DISPLAY_LINES ? items_count : DISPLAY_LINES;
	}

	void Menu::show() {
		USB_TRACE("Menu::show()\r\n");
		// buffer is one byte shorter (we are printing from position 1, not 0)
		char buffer[DISPLAY_CHARS];
		for (uint8_t i = 0; i < max_items; ++i) {
			lcd.setCursor(0, i);
			if (i == cursor_position)
				lcd.write('>');
			else
				lcd.write(' ');
			items[i + menu_offset]->get_menu_label(buffer, sizeof(buffer));
			USB_TRACE(">");
			USB_TRACE(buffer);
			USB_TRACE("<\r\n");
			lcd.print(buffer);
		}
	}

	void Menu::event_control_up() {
		USB_TRACE("Menu::event_control_up()\r\n");
		if (cursor_position) {
			--cursor_position;
			show();
		} else if (menu_offset) {
			--menu_offset;
			show();
		}
	}

	void Menu::event_control_down() {
		USB_TRACE("Menu::event_control_down()\r\n");
		if (cursor_position < max_items - 1) {
			++cursor_position;
			show();
		} else if (menu_offset < items_count - DISPLAY_LINES) {
			++menu_offset;
			show();
		}
	}


	// UI::Bool
	Bool::Bool(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value) :
		Base(lcd, label), value(value)
	{}

	char* Bool::get_menu_label(char* buffer, uint8_t buffer_size) {
		USB_TRACE("Bool::get_menu_label()\r\n");
		char* remain = Base::get_menu_label(buffer, buffer_size);
		// TODO "[on]" / "[off]"
		return remain;
	}


	// UI::Value
	Value::Value(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min) :
		Base(lcd, label), value(value), units(units), max_value(max), min_value(min)
	{}

	void Value::show() {
		USB_TRACE("Value::show()\r\n");
		lcd.setCursor(1, 0);
		lcd.printClear_P(label, 19, Ter::none);
		lcd.print(value, 5, 2);
		lcd.print_P(units);
	}

	void Value::event_control_up() {
		USB_TRACE("Value::event_control_up()\r\n");
		if (value < max_value) {
			value++;
			show();
		}
	}

	void Value::event_control_down() {
		USB_TRACE("Value::event_control_up()\r\n");
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
	Option::Option(LiquidCrystal_Prusa& lcd, const char* label, uint8_t& value, const char** options, uint8_t options_count) :
		Base(lcd, label), value(value), options(options), options_count(options_count)
	{
		if (value > options_count)
			value = 0;
	}

	void Option::show() {
		USB_TRACE("Option::show()\r\n");
		lcd.setCursor(1, 0);
		lcd.printClear_P(label, 19, Ter::none);
		lcd.setCursor(0, 2);
		lcd.printClear_P(pgmstr_emptystr, 20, Ter::none);
		uint8_t len = strlen_P(options[value]);
		if (value)
			len += 2;
		if (value < options_count - 1)
			len += 2;
		lcd.setCursor((20 - len) / 2, 2);
		if (value)
			lcd.print_P(pgmstr_lt);
		lcd.print_P(options[value]);
		if (value < options_count - 1)
			lcd.print_P(pgmstr_gt);
	}

	void Option::event_control_up() {
		USB_TRACE("Option::event_control_up()\r\n");
		if (value < options_count - 1) {
			value++;
			show();
		}
	}

	void Option::event_control_down() {
		USB_TRACE("Option::event_control_down()\r\n");
		if (value) {
			value--;
			show();
		}
	}


	// UI::State
	State::State(LiquidCrystal_Prusa& lcd, const char* label) :
		Base(lcd, label)
	{}

}
