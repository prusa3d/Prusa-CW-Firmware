#pragma once

#include <inttypes.h>

class SimplePrint {
public:
	SimplePrint();
	void print(uint16_t number, uint16_t denom = 10000, unsigned char filler = ' ');
	void print(uint8_t number, uint8_t denom = 100, unsigned char filler = ' ');
	void print(float);
	void printTime(uint16_t time);
	void print(const char*);
	void print_P(const char*);
	virtual void write(uint8_t c) = 0;
};
