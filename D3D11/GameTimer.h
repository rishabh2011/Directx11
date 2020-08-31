#pragma once
#include <windows.h>

class GameTimer
{
public:

	GameTimer();
	void reset();
	void tick();
	void start();
	void stop();
	float totalTime() const;
	float getDeltaTime() const;

private:

	double numSecondsPerCount{ 0.0f };
	double deltaTime{ 0.0f };

	_int64 launchTime{ 0 };
	_int64 currTime{ 0 };
	_int64 prevTime{ 0 };
	_int64 stopTime{ 0 };
	_int64 pausedTime{ 0 };

	bool stopped{ false };
};
