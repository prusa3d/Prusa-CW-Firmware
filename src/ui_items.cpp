#include "LiquidCrystal_Prusa.h"
#include "ui_items.h"
#include "defines.h"
#include "config.h"
#include "states.h"

namespace UI {

	// UI::Base
	Base::Base(const char* label, uint8_t last_char, bool menu_action) :
		label(label), last_char(last_char), menu_action(menu_action), long_press_ui_item(nullptr)
	{}

	char* Base::get_menu_label(char* buffer, uint8_t buffer_size) {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		buffer[--buffer_size] = char(0);	// end of text
		if (last_char)
			buffer[--buffer_size] = last_char;
		memset(buffer, ' ', buffer_size++);
		const char* from = label;
		uint8_t c = pgm_read_byte(from);
		while (--buffer_size && c) {
			*buffer = c;
			++buffer;
			c = pgm_read_byte(++from);
		}
	return buffer;
	}

	void Base::show() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	void Base::loop() {
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
			return event_button_short_press();
		if (events.button_long_press)
			return event_button_long_press();
		return nullptr;
	}

	void Base::event_cover_opened() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	void Base::event_cover_closed() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	void Base::event_tank_inserted() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	void Base::event_tank_removed() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	Base* Base::event_button_short_press() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
		return nullptr;
	}

	Base* Base::event_button_long_press() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		return long_press_ui_item;
	}

	void Base::event_control_up() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	void Base::event_control_down() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
	}

	bool Base::in_menu_action() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// do nothing
		return menu_action;
	}

	void Base::set_long_press_ui_item(Base *ui_item) {
		long_press_ui_item = ui_item;
	}


	// UI::Menu
	Menu::Menu(const char* label, Base* const* items, uint8_t items_count) :
		Base(label), items(items), items_count(items_count), menu_offset(0), cursor_position(0)
	{
		max_items = items_count < DISPLAY_LINES ? items_count : DISPLAY_LINES;
	}

	void Menu::show() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		// buffer is one byte shorter (we are printing from position 1, not 0)
		char buffer[DISPLAY_CHARS];
		for (uint8_t i = 0; i < max_items; ++i) {
			lcd.setCursor(0, i);
			if (i == cursor_position)
				lcd.write('>');
			else
				lcd.write(' ');
			items[i + menu_offset]->get_menu_label(buffer, sizeof(buffer));
/*
			USB_PRINT(" >");
			USB_PRINT(buffer);
			USB_PRINTLN("<");
*/
			lcd.print(buffer);
		}
	}

	void Menu::event_tank_inserted() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		show();
	}

	void Menu::event_tank_removed() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		show();
	}

	Base* Menu::event_button_short_press() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (items[menu_offset + cursor_position]->in_menu_action()) {
			show();
			return nullptr;
		} else {
			return items[menu_offset + cursor_position];
		}
	}

	void Menu::event_control_up() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (cursor_position < max_items - 1) {
			++cursor_position;
			show();
		} else if (menu_offset < items_count - DISPLAY_LINES) {
			++menu_offset;
			show();
		}
	}

	void Menu::event_control_down() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (cursor_position) {
			--cursor_position;
			show();
		} else if (menu_offset) {
			--menu_offset;
			show();
		}
	}


	// UI::Value
	Value::Value(const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min) :
		Base(label), units(units), value(value), max_value(max), min_value(min)
	{}

	void Value::show() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		lcd.print_P(label, 1, 0);
		lcd.print(value, 5, 2);
		lcd.print_P(units);
	}


	Base* Value::event_button_short_press() {
		write_config();
		return this;
	}

	void Value::event_control_up() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (value < max_value) {
			value++;
			show();
		}
	}

	void Value::event_control_down() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (value > min_value) {
			value--;
			show();
		}
	}

	X_of_ten::X_of_ten(const char* label, uint8_t& value) :
		Value(label, value, pgmstr_xoften, 10)
	{}

	Minutes::Minutes(const char* label, uint8_t& value, uint8_t max) :
		Value(label, value, pgmstr_minutes, max)
	{}

	Percent::Percent(const char* label, uint8_t& value, uint8_t min) :
		Value(label, value, pgmstr_percent, 100, min)
	{}

	Temperature::Temperature(const char* label, uint8_t& value) :
		Value(label, value, pgmstr_celsius, MAX_TARGET_TEMP_C, MIN_TARGET_TEMP_C)
	{}

	// config may be readed after constructor
	void Temperature::init(bool SI) {
		if (SI) {
			units = pgmstr_celsius;
			max_value = MAX_TARGET_TEMP_C;
			min_value = MIN_TARGET_TEMP_C;
		} else {
			units = pgmstr_fahrenheit;
			max_value = MAX_TARGET_TEMP_F;
			min_value = MIN_TARGET_TEMP_F;
		}
	}

	void Temperature::units_change(bool SI) {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		init(SI);
		if (SI) {
			value = round(fahrenheit2celsius(value));
		} else {
			value = round(celsius2fahrenheit(value));
		}
	}


	// UI::Bool
	Bool::Bool(const char* label, uint8_t& value, const char* true_text, const char* false_text) :
		Base(label, 0), true_text(true_text), false_text(false_text), value(value)
	{}

	char* Bool::get_menu_label(char* buffer, uint8_t buffer_size) {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		char* end = Base::get_menu_label(buffer, buffer_size);
		const char* from = value ? true_text : false_text;
		uint8_t c = pgm_read_byte(from);
		while (buffer + buffer_size > ++end && c) {
			*end = c;
			c = pgm_read_byte(++from);
		}
		return end;
	}

	bool Bool::in_menu_action() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		value ^= 1;
		write_config();
		return true;
	}


	// UI::Option
	Option::Option(const char* label, uint8_t& value, const char** options, uint8_t options_count) :
		Base(label), value(value), options(options), options_count(options_count)
	{
		if (value > options_count)
			value = 0;
	}

	void Option::show() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		lcd.print_P(label, 1, 0);
		lcd.clearLine(2);
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

	Base* Option::event_button_short_press() {
		write_config();
		return this;
	}

	void Option::event_control_up() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (value < options_count - 1) {
			value++;
			show();
		}
	}

	void Option::event_control_down() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (value) {
			value--;
			show();
		}
	}


	// UI::State
	State::State(const char* label, States::Base* state, States::Base* state_long_press, Base* menu_short_press_running, Base* menu_short_press_finished) :
		Base(label, PLAY_CHAR),
		state(state),
		state_long_press(state_long_press),
		menu_short_press_running(menu_short_press_running),
		menu_short_press_finished(menu_short_press_finished),
		old_title(nullptr),
		old_message(nullptr),
		us_last(0),
		loop_count(0)
	{}

	void State::show() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		old_title = nullptr;
		old_message = nullptr;
		loop_count = 0;
		us_last = 0;
		States::change(state);
	}

	void State::loop() {
		const char* tmp_str = States::active_state->get_title();
		if (tmp_str != old_title) {
			lcd.print_P(tmp_str, 1, 0);
			old_title = tmp_str;
		}
		tmp_str = States::active_state->get_message();
		if (tmp_str) {
			if (tmp_str != old_message) {
				lcd.print_P(tmp_str, 1, 2);
				old_message = tmp_str;
				lcd.print_P(pgmstr_space, 19, 0);
			}
		} else {
			// spinner
			lcd.setCursor(19, 0);
			uint8_t c = pgm_read_byte(pgmstr_progress + loop_count);
			lcd.write(c);
			unsigned long us_now = millis();
			if (us_now - us_last > 100) {
				us_last = us_now;
				if (++loop_count >= sizeof(pgmstr_progress)) {
					loop_count = 0;
				}
			}
		}
	}

	Base* State::event_button_short_press() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		if (old_message) {
			States::change(state_long_press);
			return menu_short_press_finished;
		} else {
			return menu_short_press_running;
		}
	}

	Base* State::event_button_long_press() {
		USB_PRINTLN(__PRETTY_FUNCTION__);
		States::change(state_long_press);
		return Base::event_button_long_press();
	}
}
