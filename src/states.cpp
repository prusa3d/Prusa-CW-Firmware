#include "states.h"
#include "defines.h"

namespace States {

	/*** states definitions ***/
	Base menu(pgmstr_emptystr);
	Confirm confirm(pgmstr_finished, pgmstr_press2continue);
	Confirm error(pgmstr_emptystr, pgmstr_emptystr);
	Washing washing(pgmstr_washing, config.fans_washing_speed, &config.washing_run_time, &confirm);
	// FIXME - would be better to set PI regulator and manage heater for drying/curing?
	Timer_heater drying(pgmstr_drying, config.fans_drying_speed, &config.drying_run_time, &confirm);
	Curing curing(pgmstr_curing, config.fans_curing_speed, &config.curing_run_time, &confirm);
	Timer_heater resin(pgmstr_heating, config.fans_menu_speed, &config.resin_preheat_run_time, &confirm, &config.resin_target_temp);
	uint8_t max_warmup_run_time = MAX_WARMUP_RUN_TIME;
	Warmup warmup_print(pgmstr_warmup, &max_warmup_run_time, nullptr, &config.target_temp);
	Warmup warmup_resin(pgmstr_warmup, &max_warmup_run_time, &resin, &config.resin_target_temp);
	Cooldown cooldown(pgmstr_cooldown, &confirm);

	Test_heater selftest_heater(pgmstr_heater_test, config.fans_drying_speed, &confirm);
	Test_uvled selftest_uvled(pgmstr_led_test, config.fans_curing_speed, &selftest_heater);
	Test_fans selftest_fans(pgmstr_fans_test, &selftest_uvled);
	Test_rotation selftest_rotation(pgmstr_rotation_test, &selftest_fans);
	Test_switch selftest_tank(pgmstr_ipatank_test, pgmstr_remove_tank, pgmstr_insert_tank, hw.is_tank_inserted, &selftest_rotation);
	Test_switch selftest_cover(pgmstr_cover_test, pgmstr_open_cover, pgmstr_close_cover, hw.is_cover_closed, &selftest_tank);


	/*** states data ***/
	Base* active_state = &menu;

	void init() {
		active_state->start();
	}

	void loop(Events& events) {
		active_state->process_events(events);
		Base* new_state = active_state->loop();
		if (new_state) {
			change(new_state);
		}
	}

	void change(Base* new_state) {
		active_state->stop();
		active_state = new_state;
		active_state->start();
	}

}
