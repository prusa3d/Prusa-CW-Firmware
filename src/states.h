#pragma once

#include "states_items.h"

namespace States {

	/*** states definitions ***/
	extern Base* active_state;

	extern Base menu;
	extern Confirm confirm;
	extern Confirm error;
	extern Base washing;
	extern Base drying;
	extern Base curing;
	extern Base resin;
	extern Warmup warmup_print;
	extern Warmup warmup_resin;
	extern Cooldown cooldown;
	extern Test_switch selftest_cover;

	void init();
	void loop(uint8_t events);
	void change(Base* new_state);

}
