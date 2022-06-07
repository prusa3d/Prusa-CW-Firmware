#pragma once

#include <Arduino.h>

#define COUNTIMER_MAX_MINUTES_SECONDS 59

class Countimer {
public:
	Countimer();

	// Set up counter time(hours, minutes, seconds and type) for existing timer.
	void setCounter(uint8_t hours, uint8_t minutes, uint8_t seconds, bool countUp);

	// Set up counter time in seconds and type for existing timer.
	void setCounterInSeconds(uint16_t seconds, bool countUp);

	// Returns timer's current hours.
	uint8_t getCurrentHours();

	// Returns timer's current minutes.
	uint8_t getCurrentMinutes();

	// Returns timer's current seconds.
	uint8_t getCurrentSeconds();

	// Returns timer's current time in seconds.
	uint16_t getCurrentTimeInSeconds();

	// Returns true if counter is completed, otherwise returns false.
	bool isCounterCompleted();

	// Returns true if counter is still running, otherwise returns false.
	bool isCounterRunning();

	// Returns true if timer is stopped, otherwise returns false.
	bool isStopped();

	// Run timer. This is main method.
	// If you want to start timer after run, you have to invoke start() method.
	void run();

	// Starting timer.
	void start();

	// Stopping timer.
	void stop();

	// Pausing timer.
	void pause();

	// Restart timer.
	void restart();

private:
	// Counting up timer.
	void countDown();

	// Counting down timer.
	void countUp();

	uint32_t _interval;
	uint32_t _previousMillis;

	// Stores current counter value in milliseconds.
	uint32_t _currentCountTime;
	uint32_t _startCountTime;

	// Stores cached user's time.
	uint32_t _countTime;

	// Function to execute when timer is complete.
	bool _isCounterCompleted;
	bool _isStopped;
	bool _countUp;
};
