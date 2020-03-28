#pragma once

#include <stdint.h>

class thermistor
{
  public:
    thermistor(uint8_t pin, uint8_t sensorNumber);
    float analog2temp();
  private:
    uint8_t _pin;
    uint8_t _sensorNumber;
};
