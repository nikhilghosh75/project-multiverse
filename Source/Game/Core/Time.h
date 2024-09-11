#pragma once

class Time
{
public:
	static float GetTime();

	static float GetDeltaTime();

	static void Update();

	static void Initialize();

private:
	static inline float time;
	static inline float deltaTime;
};