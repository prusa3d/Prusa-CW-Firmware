//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#include "MenuList.h"
#include "Main.h"
#include <stdint.h>

static const int rows = 4;

static uint8_t menu_offset = 0;
static uint8_t cursor_position = 0;

static uint_least8_t count_visible(const Scrolling_items &items)
{
    uint_least8_t visible_items = 0;
    for(auto item : items)
    {
        if(item.visible) ++visible_items;
    }

    return visible_items;
}

static uint_least8_t first_visible(const Scrolling_items &items, uint_least8_t pos)
{
    uint_least8_t first_visible = 0;
    const uint_least8_t num_items = sizeof(Scrolling_items)/sizeof(Scrolling_item);

    for (uint_least8_t i = 0; i < pos; ++i)
    {
        while ((first_visible < (num_items - 2)) && !items[first_visible].visible)
        {
            ++first_visible;
        }
        ++first_visible;
    }
    return first_visible;
}

//! @brief Print scrolling list
//!
//! Prints visible items only.
//! @param[in] items
//! @return Zero indexed selected item
uint_least8_t scrolling_list(const Scrolling_items &items)
{
    const uint_least8_t visible_items = count_visible(items);
    const uint_least8_t columns = 20;
    const uint_least8_t cursor_columns = 1;

    if (menu_offset > (visible_items - rows)) menu_offset = 0;


    if (rotary_diff > 128)
    {
        if ((cursor_position < (rows - 1)) && (cursor_position < visible_items - 1))
        {
            ++cursor_position;
        }
        else if (menu_offset < (visible_items - rows))
        {
            ++menu_offset;
        }
    }
    else if (rotary_diff < 128)
    {
        if (cursor_position)
        {
            --cursor_position;
        }
        else if (menu_offset)
        {
            --menu_offset;
        }
    }

    uint_least8_t visible_index = first_visible(items, menu_offset);
    const uint_least8_t num_items = sizeof(Scrolling_items)/sizeof(Scrolling_item);

    for (uint_least8_t line = 0; line < rows; ++line)
    {
        // (visible_index < num_items) allows visible_index to become equal num_items,
        // but such item is not shown nor accessed in next step.
        while((visible_index < num_items) && (!items[visible_index].visible)) ++visible_index;

        lcd.setCursor(cursor_columns, line);

        if (visible_index < num_items)
        {
            lcd.printClear(items[visible_index].caption, columns - cursor_columns, items[visible_index].last_symbol);
        }
        ++visible_index;
    }

    print_menu_cursor(cursor_position);
    return (cursor_position + menu_offset);
}

void scrolling_list_set(uint8_t index)
{
    if (index >= rows)
    {
        menu_offset = index - (rows - 1);
        index -= menu_offset;
    } else
    {
        menu_offset = 0;
    }
    cursor_position = index;
}
