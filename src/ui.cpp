#include "LiquidCrystal_Prusa.h"
#include "defines.h"
#include "config.h"
#include "ui.h"
#include "states.h"

namespace UI {

	/*** menu definitions ***/
	Base back(pgmstr_back, BACK_CHAR);
	Base stop(pgmstr_stop, STOP_CHAR);

	// move all following definitions to init() to save RAM instead of PROGMEM

	// run time menu
	Minutes curing_run_time(pgmstr_curing_run_time, config.curing_run_time, MAX_CURING_RUNTIME);
	Minutes drying_run_time(pgmstr_drying_run_time, config.drying_run_time, MAX_DRYING_RUNTIME);
	Minutes washing_run_time(pgmstr_washing_run_time, config.washing_run_time, MAX_WASHING_RUNTIME);
	Minutes resin_preheat_run_time(pgmstr_resin_preheat_time, config.resin_preheat_run_time, MAX_PREHEAT_RUNTIME);
	Base* const run_time_items[] PROGMEM = {&back, &curing_run_time, &drying_run_time, &washing_run_time, &resin_preheat_run_time};
	Menu run_time_menu(pgmstr_run_time, run_time_items, COUNT_ITEMS(run_time_items));

	// speed menu
	X_of_ten curing_speed(pgmstr_curing_speed, config.curing_speed);
	X_of_ten washing_speed(pgmstr_washing_speed, config.washing_speed);
	Base* const speed_items[] PROGMEM = {&back, &curing_speed, &washing_speed};
	Menu speed_menu(pgmstr_rotation_speed, speed_items, COUNT_ITEMS(speed_items));

	// temperatore menu
	Bool heat_to_target_temp(pgmstr_warmup, config.heat_to_target_temp);
	Temperature target_temp(pgmstr_drying_warmup_temp, config.target_temp);
	Temperature resin_target_temp(pgmstr_resin_preheat_temp, config.resin_target_temp);
	Temperature* const SI_changed[] PROGMEM = {&target_temp, &resin_target_temp};
	SI_switch SI_unit_system(pgmstr_units, config.SI_unit_system, SI_changed, COUNT_ITEMS(SI_changed));
	Base* const temperature_items[] PROGMEM = {&back, &heat_to_target_temp, &target_temp, &resin_target_temp, &SI_unit_system};
	Menu temperature_menu(pgmstr_temperatures, temperature_items, COUNT_ITEMS(temperature_items));

	// sound menu
	Bool sound_response(pgmstr_control_echo, config.sound_response);
	const char* const finish_beep_options[] PROGMEM = {pgmstr_none, pgmstr_once, pgmstr_continuous};
	Option finish_beep(pgmstr_finish_beep, config.finish_beep_mode, finish_beep_options, COUNT_ITEMS(finish_beep_options));
	Base* const sound_items[] PROGMEM = {&back, &sound_response, &finish_beep};
	Menu sound_menu(pgmstr_sound, sound_items, COUNT_ITEMS(sound_items));

	// fans curing speed
	Percent fan1_curing_speed(pgmstr_fan1_curing_speed, config.fans_curing_speed[0], MIN_FAN_SPEED);
	Percent fan2_curing_speed(pgmstr_fan2_curing_speed, config.fans_curing_speed[1], MIN_FAN_SPEED);
	Base* const fans_curing_speed[] PROGMEM = {&back, &fan1_curing_speed, &fan2_curing_speed};
	Menu fans_curing_menu(pgmstr_fans_curing, fans_curing_speed, COUNT_ITEMS(fans_curing_speed));

	// fans drying speed
	Percent fan1_drying_speed(pgmstr_fan1_drying_speed, config.fans_drying_speed[0], MIN_FAN_SPEED);
	Percent fan2_drying_speed(pgmstr_fan2_drying_speed, config.fans_drying_speed[1], MIN_FAN_SPEED);
	Base* const fans_drying_speed[] PROGMEM = {&back, &fan1_drying_speed, &fan2_drying_speed};
	Menu fans_drying_menu(pgmstr_fans_drying, fans_drying_speed, COUNT_ITEMS(fans_drying_speed));

	// fans washing speed
	Percent fan1_washing_speed(pgmstr_fan1_washing_speed, config.fans_washing_speed[0], MIN_FAN_SPEED);
	Percent fan2_washing_speed(pgmstr_fan2_washing_speed, config.fans_washing_speed[1], MIN_FAN_SPEED);
	Base* const fans_washing_speed[] PROGMEM = {&back, &fan1_washing_speed, &fan2_washing_speed};
	Menu fans_washing_menu(pgmstr_fans_washing, fans_washing_speed, COUNT_ITEMS(fans_washing_speed));

	// fans menu speed
	Percent_with_action fan1_menu_speed(pgmstr_fan1_menu_speed, config.fans_menu_speed[0], MIN_FAN_SPEED, hw.set_fan1_duty);
	Percent_with_action fan2_menu_speed(pgmstr_fan2_menu_speed, config.fans_menu_speed[1], MIN_FAN_SPEED, hw.set_fan2_duty);
	Base* const fans_menu_speed[] PROGMEM = {&back, &fan1_menu_speed, &fan2_menu_speed};
	Menu fans_menu_menu(pgmstr_fans_menu, fans_menu_speed, COUNT_ITEMS(fans_menu_speed));

	// fans menu
	Base* const fans_items[] PROGMEM = {&back, &fans_curing_menu, &fans_drying_menu, &fans_washing_menu, &fans_menu_menu};
	Menu fans_menu(pgmstr_fans, fans_items, COUNT_ITEMS(fans_items));

	// info menu
	SN serial_number(pgmstr_sn);
	Text fw_version(pgmstr_fw_version);
	Text build_nr(pgmstr_build_nr);
	Text fw_hash(pgmstr_fw_hash);
#if FW_LOCAL_CHANGES
	Text workspace_dirty(pgmstr_workspace_dirty);
	Base* const info_items[] PROGMEM = {&back, &serial_number, &fw_version, &build_nr, &fw_hash, &workspace_dirty};
#else
	Base* const info_items[] PROGMEM = {&back, &serial_number, &fw_version, &build_nr, &fw_hash};
#endif
	Menu info_menu(pgmstr_information, info_items, COUNT_ITEMS(info_items));

	// config menu
	const char* const curing_machine_mode_options[] PROGMEM = {pgmstr_drying_curing, pgmstr_curing, pgmstr_drying};
	Option curing_machine_mode(pgmstr_run_mode, config.curing_machine_mode, curing_machine_mode_options, COUNT_ITEMS(curing_machine_mode_options));
	Percent led_intensity(pgmstr_led_intensity, config.led_intensity, MIN_LED_INTENSITY);
	Percent_with_action lcd_brightness(pgmstr_lcd_brightness, config.lcd_brightness, MIN_LCD_BRIGHTNESS, lcd.setBrightness);
	Base* const config_items[] PROGMEM = {&back, &speed_menu, &curing_machine_mode, &temperature_menu, &sound_menu, &lcd_brightness, &info_menu};
	Menu config_menu(pgmstr_settings, config_items, COUNT_ITEMS(config_items));

	// run menu
	Pause pause(&back);
	Base* const run_items[] PROGMEM = {&pause, &stop, &back};
	Menu run_menu(pgmstr_emptystr, run_items, COUNT_ITEMS(run_items));

	// hold platform function
	#ifdef CW1S
		Base* const hold_platform_items[] PROGMEM = {&back};
		Hold_platform hold_platform_menu(pgmstr_hold_platform, hold_platform_items, COUNT_ITEMS(hold_platform_items));
	#endif

	// home menu
	Do_it do_it(config.curing_machine_mode, &run_menu);
	State resin_preheat(pgmstr_resin_preheat, &States::warmup_resin, &run_menu);
	#ifdef CW1S
		Base* const home_items[] PROGMEM = {&do_it, &resin_preheat, &run_time_menu, &config_menu, &hold_platform_menu};
	#else
		Base* const home_items[] PROGMEM = {&do_it, &resin_preheat, &run_time_menu, &config_menu};
	#endif
	Menu home_menu(pgmstr_emptystr, home_items, COUNT_ITEMS(home_items));

	// hw menu
	Live_value<uint16_t> fan1_rpm(pgmstr_fan1_rpm, hw.fan_rpm[0]);
	Live_value<uint16_t> fan2_rpm(pgmstr_fan2_rpm, hw.fan_rpm[1]);
	Live_value<uint16_t> fan3_rpm(pgmstr_fan3_rpm, hw.fan_rpm[2]);
	Live_value<float> chamber_temp(pgmstr_chamber_temp, hw.chamber_temp);
	Live_value<float> uvled_temp(pgmstr_uvled_temp, hw.uvled_temp);
	Base* const hw_items[] PROGMEM = {&back, &fan1_rpm, &fan2_rpm, &fan3_rpm, &chamber_temp, &uvled_temp};
	Menu_self_redraw hw_menu(pgmstr_emptystr, hw_items, COUNT_ITEMS(hw_items), MENU_REDRAW_US);

	// advanced menu
	State cooldown(pgmstr_cooldown, &States::cooldown, &hw_menu);
	State selftest(pgmstr_selftest, &States::selftest_cover, nullptr);
	Base* const advanced_items[] PROGMEM = {&back, &fans_menu, &led_intensity, &cooldown, &selftest};
	Menu advanced_menu(pgmstr_emptystr, advanced_items, COUNT_ITEMS(advanced_items));


	/*** menu data ***/
	Base* menu_stack[MAX_MENU_DEPTH];
	uint8_t menu_depth = 0;
	Base* active_menu = &home_menu;

	void init() {
		for (uint8_t i = 0; i < COUNT_ITEMS(SI_changed); ++i) {
			SI_changed[i]->init(config.SI_unit_system);
		}
		home_menu.set_long_press_ui_item(&curing_machine_mode);
		info_menu.set_long_press_ui_item(&hw_menu);
		run_menu.set_long_press_ui_item(&hw_menu);
		config_menu.set_long_press_ui_item(&advanced_menu);
		active_menu->invoke();
		active_menu->show();
	}

	void loop(uint8_t events) {
		active_menu->loop();
		Base* new_menu = active_menu->process_events(events);
		if (new_menu == &stop || new_menu == &back || new_menu == active_menu || States::active_state->is_finished()) {
			if (menu_depth) {
				do {
					active_menu->leave();
					active_menu = menu_stack[--menu_depth];
				} while (new_menu == &stop && menu_depth);
				lcd.clear();
				active_menu->show();
			} else {
				USB_PRINTLNP("ERROR: back at menu depth 0!");
			}
		} else if (new_menu) {
			if (menu_depth < MAX_MENU_DEPTH) {
				menu_stack[menu_depth++] = active_menu;
				active_menu = new_menu;
				lcd.clear();
				active_menu->invoke();
				active_menu->show();
			} else {
				USB_PRINTLNP("ERROR: MAX_MENU_DEPTH reached!");
			}
		}
	}

}
