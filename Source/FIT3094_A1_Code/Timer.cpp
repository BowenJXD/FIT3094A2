#include "Timer.h"

Timer::Timer(float Duration, int Loop)
{
	this->Duration = Duration;
	this->TimeRemaining = Duration;
	this->Loop = Loop;
	this->LoopRemaining = Loop;
}

bool Timer::Tick(float DeltaTime)
{
	if (LoopRemaining == 0) {
		return false;
	}
	
	TimeRemaining -= DeltaTime;
	if (TimeRemaining <= 0) {
		TimeRemaining = Duration;
		if (Loop != -1)
		{
			LoopRemaining--;
		}
		return true;
	}
	return false;
}
