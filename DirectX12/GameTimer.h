#pragma once

class CGameTimer {
public:
	CGameTimer();

	float GetGameTime() const;
	float GetDeltaTime() const;

	void Reset();

	void Start();
	void Stop();
	void Tick();

private:
	double SecondsPerCount;
	double DeltaTime;

	__int64 BaseTime;
	__int64 PausedTime;
	__int64 StopTime;
	__int64 PrevTime;
	__int64 CurrTime;

	bool Stopped;
};