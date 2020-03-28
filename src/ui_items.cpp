#include "LiquidCrystal_Prusa.h"
#include "defines.h"
#include "config.h"
#include "ui_items.h"
#include "states.h"

namespace UI {

	// UI::Base
	Base::Base(const char* label, uint8_t last_char) :
		label(label), last_char(last_char)
	{}

	char* Base::get_menu_label(char* buffer, uint8_t buffer_size) {
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
	}

	void Base::loop() {
	}

	void Base::invoke() {
	}

	void Base::leave() {
	}

	Base* Base::process_events(Events& events) {
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
	}

	void Base::event_cover_closed() {
	}

	void Base::event_tank_inserted() {
	}

	void Base::event_tank_removed() {
	}

	Base* Base::event_button_short_press() {
		return nullptr;
	}

	Base* Base::event_button_long_press() {
		return nullptr;
	}

	void Base::event_control_up() {
	}

	void Base::event_control_down() {
	}

	Base* Base::in_menu_action() {
		// go to next level
		return nullptr;
	}


	// UI::Text
	Text::Text(const char* label) :
		Base(label, 0)
	{}

	Base* Text::in_menu_action() {
		// stay in menu
		return this;
	}


	// UI:SN
	SN::SN(const char* label) :
		Text(label)
	{}

	char* SN::get_menu_label(char* buffer, uint8_t buffer_size) {
		strncpy_P(buffer, pgmstr_sn, buffer_size);
		// sizeof() != strlen()
		uint8_t bs = buffer_size - (sizeof(pgmstr_sn) - 1);
		return Base::get_menu_label(buffer + sizeof(pgmstr_sn) - 1, bs < SN_LENGTH+1 ? bs : SN_LENGTH+1);
	}


	// UI::Menu
	Menu::Menu(const char* label, Base* const* items, uint8_t items_count) :
		Base(label), items(items), long_press_ui_item(nullptr), items_count(items_count), menu_offset(0), cursor_position(0)
	{
		max_items = items_count < DISPLAY_LINES ? items_count : DISPLAY_LINES;
	}

	void Menu::invoke() {
		cursor_position = 0;
		Base::invoke();
	}

	void Menu::show() {
		// buffer is one byte shorter (we are printing from position 1, not 0)
		char buffer[DISPLAY_CHARS];
		for (uint8_t i = 0; i < max_items; ++i) {
			lcd.setCursor(0, i);
			if (i == cursor_position)
				lcd.write('>');
			else
				lcd.write(' ');
			items[i + menu_offset]->get_menu_label(buffer, sizeof(buffer));
			lcd.print(buffer);
		}
	}

	void Menu::event_tank_inserted() {
		show();
	}

	void Menu::event_tank_removed() {
		show();
	}

	Base* Menu::event_button_short_press() {
		Base* menu_action = items[menu_offset + cursor_position]->in_menu_action();
		if (menu_action) {
			if (menu_action == items[menu_offset + cursor_position]) {
				show();
				return nullptr;
			} else {
				return menu_action;
			}
		} else {
			return items[menu_offset + cursor_position];
		}
	}

	Base* Menu::event_button_long_press() {
		return long_press_ui_item;
	}

	void Menu::event_control_up() {
		if (cursor_position < max_items - 1) {
			++cursor_position;
			show();
		} else if (menu_offset < items_count - DISPLAY_LINES) {
			++menu_offset;
			show();
		}
	}

	void Menu::event_control_down() {
		if (cursor_position) {
			--cursor_position;
			show();
		} else if (menu_offset) {
			--menu_offset;
			show();
		}
	}

	void Menu::set_long_press_ui_item(Base *ui_item) {
		long_press_ui_item = ui_item;
	}


	// UI::Value
	Value::Value(const char* label, uint8_t& value, const char* units, uint8_t max, uint8_t min) :
		Base(label), units(units), value(value), max_value(max), min_value(min)
	{}

	void Value::show() {
		lcd.print_P(label, 1, 0);
		lcd.print(value, 5, 2);
		lcd.print_P(units);
	}


	Base* Value::event_button_short_press() {
		write_config();
		return this;
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
		init(SI);
		if (SI) {
			value = round(fahrenheit2celsius(value));
		} else {
			value = round(celsius2fahrenheit(value));
		}
	}


	// UI::LcdBrightness
	LcdBrightness::LcdBrightness(const char* label, uint8_t& value) :
		Percent(label, value, 5)
	{}

	void LcdBrightness::event_control_up() {
		Percent::event_control_up();
		lcd.setBrightness(value);
	}

	void LcdBrightness::event_control_down() {
		Percent::event_control_down();
		lcd.setBrightness(value);
	}


	// UI::Bool
	Bool::Bool(const char* label, uint8_t& value, const char* true_text, const char* false_text) :
		Base(label, 0), true_text(true_text), false_text(false_text), value(value)
	{}

	char* Bool::get_menu_label(char* buffer, uint8_t buffer_size) {
		char* end = Base::get_menu_label(buffer, buffer_size);
		const char* from = value ? true_text : false_text;
		uint8_t c = pgm_read_byte(from);
		while (buffer + buffer_size > ++end && c) {
			*end = c;
			c = pgm_read_byte(++from);
		}
		return end;
	}

	Base* Bool::in_menu_action() {
		value ^= 1;
		write_config();
		return this;
	}


	// UI::SI_switch
	SI_switch::SI_switch(const char* label, uint8_t& value, Temperature* const* to_change, uint8_t to_change_count) :
		Bool(label, value, pgmstr_celsius_units, pgmstr_fahrenheit_units), to_change(to_change), to_change_count(to_change_count)
	{}

	Base* SI_switch::in_menu_action() {
		for (uint8_t i = 0; i < to_change_count; ++i) {
			to_change[i]->units_change(value^1);
		}
		Bool::in_menu_action();
		return this;
	}


	// UI::Option
	Option::Option(const char* label, uint8_t& value, const char** options, uint8_t options_count) :
		Base(label), value(value), options(options), options_count(options_count)
	{
		if (value > options_count)
			value = 0;
	}

	void Option::show() {
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
		if (value < options_count - 1) {
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
	State::State(const char* label, States::Base* state, Base* state_menu) :
		Base(label, PLAY_CHAR),
		state(state),
		state_menu(state_menu),
		old_title(nullptr),
		old_message(nullptr),
		old_time(UINT16_MAX),
		spin_us_last(0),
		bound_us_last(0),
		spin_count(0)
	{}

	void State::show() {
		old_title = nullptr;
		old_message = nullptr;
		old_time = UINT16_MAX;
		spin_us_last = 0;
		bound_us_last = 0;
		spin_count = 0;
		Base::show();
	}

	void State::loop() {
		const char* tmp_str = States::active_state->get_title();
		if (tmp_str != old_title) {
			lcd.clear();
			lcd.print_P(tmp_str, 1, 0);
			old_title = tmp_str;
		}
		tmp_str = States::active_state->get_message();
		if (tmp_str) {
			if (tmp_str != old_message) {
				old_message = tmp_str;
				lcd.print_P(tmp_str, 1, 2);
			}
		} else {
			unsigned long us_now = millis();
			// spinner
			if (!States::active_state->is_paused()) {
				lcd.setCursor(19, 0);
				uint8_t c = pgm_read_byte(pgmstr_progress + spin_count);
				lcd.write(c);
				if (us_now - spin_us_last > 100) {
					spin_us_last = us_now;
					if (++spin_count >= sizeof(pgmstr_progress)) {
						spin_count = 0;
					}
				}
			}
			if (bound_us_last && us_now - bound_us_last > 1000) {
				clear_time_boundaries();
				bound_us_last = 0;
			}
			// time
			uint16_t time = States::active_state->get_time();
			if (time != UINT16_MAX && time != old_time) {
				old_time = time;
				lcd.printTime(time, LAYOUT_TIME_X, LAYOUT_TIME_Y);
				// temperature
				float temp = States::active_state->get_temperature();
				if (temp > 0) {
					lcd.print(temp, LAYOUT_TEMP_X, LAYOUT_TEMP_Y);
					lcd.print_P(config.SI_unit_system ? pgmstr_celsius : pgmstr_fahrenheit);
				}
			}
		}
	}

	void State::invoke() {
		States::change(state);
		Base::invoke();
	}

	void State::leave() {
		States::change(&States::menu);
		Base::invoke();
	}

	void State::event_cover_opened() {
		old_time = UINT16_MAX;
		Base::event_cover_opened();
	}

	void State::event_cover_closed() {
		old_time = UINT16_MAX;
		Base::event_cover_closed();
	}

	void State::event_tank_inserted() {
		old_time = UINT16_MAX;
		Base::event_tank_inserted();
	}

	void State::event_tank_removed() {
		old_time = UINT16_MAX;
		Base::event_tank_removed();
	}

	Base* State::event_button_short_press() {
		if (States::active_state->is_menu_available()) {
			return state_menu;
		}
		return nullptr;
	}

	void State::event_control_up() {
		if (States::active_state->get_time() != UINT16_MAX) {
			clear_time_boundaries();
			const char* symbol = States::active_state->increase_time();
			if (symbol) {
				lcd.print_P(symbol, LAYOUT_TIME_GT, LAYOUT_TIME_Y);
				bound_us_last = millis();
			}
		}
	}

	void State::event_control_down() {
		if (States::active_state->get_time() != UINT16_MAX) {
			clear_time_boundaries();
			const char* symbol = States::active_state->decrease_time();
			if (symbol) {
				lcd.print_P(symbol, LAYOUT_TIME_LT, LAYOUT_TIME_Y);
				bound_us_last = millis();
			}
		}
	}

	void State::clear_time_boundaries() {
		lcd.print_P(pgmstr_double_space, LAYOUT_TIME_GT, LAYOUT_TIME_Y);
		lcd.print_P(pgmstr_double_space, LAYOUT_TIME_LT, LAYOUT_TIME_Y);
	}


	// UI::Do_it
	Do_it::Do_it(const char* label, uint8_t& curing_machine_mode, Base* state_menu) :
		State(label, nullptr, state_menu), curing_machine_mode(curing_machine_mode)
	{}

	char* Do_it::get_menu_label(char* buffer, uint8_t buffer_size) {
		if (hw.is_tank_inserted()) {
			label = pgmstr_washing;
		} else {
			switch (curing_machine_mode) {
				case 2:
					label = pgmstr_drying;
					break;
				case 1:
					label = pgmstr_curing;
					break;
				default:
					label = pgmstr_drying_curing;
					break;
			}
		}
		return State::get_menu_label(buffer, buffer_size);
	}

	void Do_it::invoke() {
		if (hw.is_tank_inserted()) {
			state = &States::washing;
		} else {
			switch (curing_machine_mode) {
				case 2:
					States::warmup_print.set_continue_to(&States::drying);
					break;
				case 1:
					States::warmup_print.set_continue_to(&States::curing);
					break;
				default:
					States::warmup_print.set_continue_to(&States::drying_curing);
					break;
			}
			state = &States::warmup_print;
		}
		State::invoke();
	}


	// UI::Pause
	Pause::Pause(Base* back) :
		Base(pgmstr_emptystr), back(back)
	{}

	char* Pause::get_menu_label(char* buffer, uint8_t buffer_size) {
		buffer[--buffer_size] = char(0);	// end of text
		memset(buffer, ' ', buffer_size++);
		const char* from = States::active_state->is_paused() ? pgmstr_continue : pgmstr_pause;
		uint8_t c = pgm_read_byte(from);
		while (--buffer_size && c) {
			*buffer = c;
			++buffer;
			c = pgm_read_byte(++from);
		}
		return buffer;
	}

	Base* Pause::in_menu_action() {
		States::active_state->pause_continue();
		return back;
	}

}
