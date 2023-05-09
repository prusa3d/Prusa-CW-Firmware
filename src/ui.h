#pragma once

#include "ui_items.h"

namespace UI {

	extern State error;

	void init();
	void set_menu(Base* new_menu);
	void loop(uint8_t events);

}
