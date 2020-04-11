#include <stdio.h>
#include "Countimer.h"

Countimer::Countimer() :
	_interval(1),
	_previousMillis(0),
	_currentCountTime(0),
	_startCountTime(0),
	_countTime(0),
	_isCounterCompleted(true),
	_isStopped(true),
	_countType(CountType::COUNT_NONE)
{ }

void Countimer::setCounter(uint8_t hours, uint8_t minutes, uint8_t seconds, CountType countType) {
	if (minutes > COUNTIMER_MAX_MINUTES_SECONDS) {
		minutes = COUNTIMER_MAX_MINUTES_SECONDS;
	}

	if (seconds > COUNTIMER_MAX_MINUTES_SECONDS) {
		seconds = COUNTIMER_MAX_MINUTES_SECONDS;
	}
	_countType = countType;
	setCounterInSeconds((hours * 3600L) + (minutes * 60L) + seconds);
}

void Countimer::setCounterInSeconds(uint16_t seconds) {
	_currentCountTime = seconds * 1000L;
	_countTime = _currentCountTime;

	if (_countType == COUNT_UP) {
		// if is count up mode, we have to start from 00:00:00;
		_currentCountTime = 0;
	}

	_startCountTime = _currentCountTime;
}

uint8_t Countimer::getCurrentHours() {
	return _currentCountTime / 1000 / 3600;
}

uint8_t Countimer::getCurrentMinutes() {
	return _currentCountTime / 1000 % 3600 / 60;
}

uint8_t Countimer::getCurrentSeconds() {
	return _currentCountTime / 1000 % 3600 % 60 % 60;
}

uint16_t Countimer::getCurrentTimeInSeconds() {
	return _currentCountTime / 1000;
}

bool Countimer::isCounterCompleted() {
	return _isCounterCompleted;
}

bool Countimer::isStopped() {
	return _isStopped;
}

void Countimer::start() {
	_isStopped = false;
	if (_isCounterCompleted) {
		_isCounterCompleted = false;
	}
}

void Countimer::pause() {
	_isStopped = true;
}

void Countimer::stop() {
	_isStopped = true;
	_isCounterCompleted = true;
	_currentCountTime = _countTime;

	if (_countType == COUNT_UP) {
		_currentCountTime = 0;		
	}
}

void Countimer::restart() {
	_currentCountTime = _startCountTime;
	_isCounterCompleted = false;
	_isStopped = false;

	start();
}

void Countimer::run() {
	// timer is running only if is not completed or not stopped.
	if (_isCounterCompleted || _isStopped) {
		return;
	}

	if (millis() - _previousMillis >= _interval) {

		if (_countType == COUNT_DOWN) {
			countDown();
		} else if (_countType == COUNT_UP) {
			countUp();
		}
		_previousMillis = millis();
	}
}

void Countimer::countDown() {
	if (_currentCountTime > 0) {
		_currentCountTime -= _interval;
	} else {
		stop();
	}
}

void Countimer::countUp() {
	if (_currentCountTime < _countTime) {
		_currentCountTime += _interval;
	} else {
		stop();
	}
}
