#include "Engine.h"

Timer::Timer(){
	QueryPerformanceFrequency(&m_freq);
}

Timer::~Timer(){

}

void Timer::createTimeStamp(TimeStamp &stamp) const{
	QueryPerformanceCounter(&stamp);
}

double Timer::getDeltaTime(const TimeStamp &stampA, const TimeStamp &stampB) const{
	return (static_cast<double>(stampB.QuadPart) - static_cast<double>(stampA.QuadPart)) /
		static_cast<double>(m_freq.QuadPart);
}
