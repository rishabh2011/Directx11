#include "GameTimer.h"

//--------------------
GameTimer::GameTimer()
{
	_int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)& countsPerSec);

	//this is needed as QueryPerformanceCounter will return number of counts
	//that have passed since the application was launched and since we are targeting
	//fps dependent rendering updates we need to convert said counts into seconds corresponding to those counts
	numSecondsPerCount = 1.0 / static_cast<double>(countsPerSec);
}

//---------------------
void GameTimer::reset()
{
	_int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	
	launchTime = currentTime;
	prevTime = currentTime;
	stopTime = 0;
	stopped = false;
}

//--------------------
void GameTimer::tick()
{
	if (stopped)
	{
		deltaTime = 0.0;
		return;
	}

	_int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	currTime = currentTime;
	deltaTime = (currTime - prevTime) * numSecondsPerCount;
	prevTime = currTime;

	// Force nonnegative. The DXSDK's CDXUTTimer mentions that if the
	// processor goes into a power save mode or we get shuffled to another
	// processor, then deltaTime can be negative.
	if (deltaTime < 0.0)
	{
		deltaTime = 0.0;
	}
}

//-----------------------------------
float GameTimer::getDeltaTime() const
{
	return static_cast<float>(deltaTime);
}

//---------------------
void GameTimer::start()
{
	if (stopped)
	{
		_int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

		pausedTime += startTime - stopTime;
		prevTime = startTime;
		stopTime = 0;
		stopped = false;
	}
}

//--------------------
void GameTimer::stop()
{
	if (!stopped)
	{
		_int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		stopTime = currentTime;
		stopped = true;
	}
}

//return time passed since app was 
//launched but exclude pause time
//--------------------------------
float GameTimer::totalTime() const
{
	if (stopped)
	{
		//considering previous pauses 
		//(stoptime - pausedTime) - launchTime gives the total time app has been active
		return static_cast<float>(((stopTime - pausedTime) - launchTime) * numSecondsPerCount);
	}
	else
	{
		return static_cast<float>(((currTime - pausedTime) - launchTime) * numSecondsPerCount);
	}
}
