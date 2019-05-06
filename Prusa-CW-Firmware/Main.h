//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#ifndef CW_MAIN_H
#define CW_MAIN_H
#include <stdint.h>
#include "PrusaLcd.h"

extern uint8_t menu_position;
extern uint8_t cursor_position;
extern uint8_t menu_offset;
extern volatile uint8_t rotary_diff;
extern PrusaLcd lcd;

void print_menu_cursor();
void print_scrolling_menu_cursor();
void lcd_print_back();
void lcd_print_right(int a);


#endif /* CW_MAIN_H */
