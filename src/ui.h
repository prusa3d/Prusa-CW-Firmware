#pragma once

#include "ui_items.h"

namespace UI {

	extern State error;
	extern Menu home_menu;
	extern Option curing_machine_mode;
	extern Option washing_mode;

	void init();
	void set_menu(Base* new_menu);
	void loop(uint8_t events);

}
