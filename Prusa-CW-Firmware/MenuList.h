//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#ifndef MENULIST_H
#define MENULIST_H

#include "PrusaLcd.h"
#include <stdint.h>

struct Scrolling_item
{
   const char *caption_P;
   bool visible;
   PrusaLcd::Terminator last_symbol;
};

uint_least8_t scrolling_list_P(const Scrolling_item items[], uint8_t item_count);

template<uint8_t n>
uint_least8_t scrolling_list_P(const Scrolling_item (&items)[n])
{
  return scrolling_list_P(items, sizeof(items)/sizeof(Scrolling_item));
}


void scrolling_list_set(uint8_t index);

#endif /* MENULIST_H */
