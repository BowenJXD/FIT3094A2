#include "CustomAction.h"

bool CustomAction::IsActionDone()
{
	return State == Finished;
}

bool CustomAction::Execute(AShip* Ship, float DeltaTime)
{
	bool Result = false;	
	if (State == NotStarted) {
		Executor = Ship;
		OnStart();
		State = Running;
	}
	if (State == Running) {
		Result = OnTick(DeltaTime);
	}
	if (State == Finished) {
		OnFinish();
	}
	
	return Result;
}
