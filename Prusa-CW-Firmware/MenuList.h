//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#ifndef MENULIST_H
#define MENULIST_H

#include <stdint.h>

struct Scrolling_item
{
   const char *caption;
   bool visible;
};

typedef Scrolling_item Scrolling_items[8];

uint_least8_t scrolling_list(const Scrolling_items &items);
void scrolling_list_reset();

#endif /* MENULIST_H */
