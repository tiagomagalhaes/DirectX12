#include "GameTimer.h"

#include <Windows.h>

CGameTimer::CGameTimer() : SecondsPerCount(0.0), DeltaTime(-1.0), BaseTime(0), PausedTime(0), PrevTime(0), CurrTime(0), Stopped(false) {
	__int64 countsPerSec;

	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	SecondsPerCount = 1.0f / (double)countsPerSec;
}

float CGameTimer::GetGameTime() const {

	if (Stopped)
		return (float)(((StopTime - PausedTime) - BaseTime) * SecondsPerCount);
	else
		return (float)(((CurrTime - PausedTime) - BaseTime) * SecondsPerCount);
}

float CGameTimer::GetDeltaTime() const {
	return (float)DeltaTime;
}

void CGameTimer::Reset() {
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	BaseTime = currTime;
	PrevTime = currTime;
	StopTime = 0;
	Stopped = false;
}

void CGameTimer::Start() {
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// If we are resuming the timer from a stopped state...
	if (Stopped) {
		// then accumulate the paused time.
		PausedTime += (startTime - StopTime);

		// since we are starting the timer back up, the current 
		// previous time is not valid, as it occurred while paused.
		// So reset it to the current time.
		PrevTime = startTime;

		// no longer stopped...
		StopTime = 0;
		Stopped = false;
	}
}

void CGameTimer::Stop() {
	if (!Stopped) {
		Stopped = true;

		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// Otherwise, save the time we stopped at, and set 
		// the Boolean flag indicating the timer is stopped.
		StopTime = currTime;
		Stopped = true;
	}
}

void CGameTimer::Tick() {
	if (Stopped) {
		DeltaTime = 0.0f;
		return;
	}

	// Get the time this frame.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	CurrTime = currTime;

	// Time difference between this frame and the previous.
	DeltaTime = (CurrTime - PrevTime) * SecondsPerCount;

	// Prepare for next frame.
	PrevTime = CurrTime;

	// Force nonnegative. The DXSDK’s CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to
	// another processor, then mDeltaTime can be negative.
	if (DeltaTime < 0.0)
		DeltaTime = 0.0;
}