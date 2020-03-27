#pragma once

#include "states_items.h"

namespace States {

	/*** states definitions ***/
	extern Base* active_state;

	extern Base menu;
	extern Confirm confirm;
	extern Confirm heater_error;
	extern Timer washing;
	extern TimerHeater drying;
	extern Curing curing;
	extern TimerHeater drying_curing;
	extern TimerHeater resin;
	extern Warmup warmup_print;
	extern Warmup warmup_resin;
	extern Base selftest_cover;

	void init();
	void loop(Events& events);
	void change(Base* new_state);

}
