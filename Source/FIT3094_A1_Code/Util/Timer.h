#pragma once

class Timer
{
public:
	float Duration;
	float TimeRemaining;
	int Loop;
	int LoopRemaining;

	Timer(float Duration, int Loop);

	/**
	 * @brief 
	 * @param DeltaTime 
	 * @return whether the timer just finished a loop
	 */
	bool Tick(float DeltaTime);
};
