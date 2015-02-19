#pragma once

using TimeStamp = LARGE_INTEGER;

class Timer{
private:
	LARGE_INTEGER m_freq;

public:
	Timer();
	~Timer();

	void createTimeStamp(TimeStamp &stamp) const;
	double getDeltaTime(const TimeStamp &stampA, const TimeStamp &stampB) const;
};
