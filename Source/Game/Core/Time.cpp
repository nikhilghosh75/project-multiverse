#include "Time.h"
#include <chrono>

std::chrono::high_resolution_clock::time_point lastTimePoint;

float Time::GetTime()
{
	return time;
}

float Time::GetDeltaTime()
{
	return deltaTime;
}

void Time::Update()
{
	std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
	int64_t microseconds = (std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTimePoint)).count();
	deltaTime = (float)microseconds / 1000000.f;

	time += deltaTime;

	lastTimePoint = currentTime;
}

void Time::Initialize()
{
	time = 0;
	lastTimePoint = std::chrono::high_resolution_clock::now();
}
