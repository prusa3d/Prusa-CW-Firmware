#include "simple_print.h"
#include <stdlib.h>
#include "Arduino.h"

SimplePrint::SimplePrint()
{}

void SimplePrint::print(uint8_t number, uint8_t denom, unsigned char filler) {
	print((uint16_t)number, (uint16_t)denom, filler);
}

void SimplePrint::print(uint16_t number, uint16_t denom, unsigned char filler) {
	div_t division;
	while (denom) {
		division = div(number, denom);
		if (division.quot || denom == 1) {
			write(division.quot + '0');
			filler = '0';
		} else {
			write(filler);
		}
		number = division.rem;
		denom /= 10;
	}
}

void SimplePrint::print(float number) {
	number += 0.05;
	uint8_t integer = (uint8_t)number;
	print(integer, 100, ' ');
	write('.');
	integer = (number - integer) * 10;
	print(integer, 1);
}

void SimplePrint::printTime(uint16_t time) {
	uint8_t min = time / 60;
	uint8_t sec = time % 60;
	print(min, 10, '0');
	write(':');
	print(sec, 10, '0');
}

void SimplePrint::print(const char *str) {
	uint8_t c;
	while ((c = *(str++))) {
		write(c);
	}
}

void SimplePrint::print_P(const char *str) {
	uint8_t c;
	while ((c = pgm_read_byte(str++))) {
		write(c);
	}
}
