#include "CustomAction.h"

bool UCustomAction::IsActionDone()
{
	return State == Finished;
}

bool UCustomAction::SetupAction(AShip* Ship)
{
	Executor = Ship;
	return true;
}

bool UCustomAction::Execute(AShip* Ship, float DeltaTime)
{
	bool Result = false;	
	if (State == NotStarted) {
		OnStart();
		State = Running;
	}
	if (State == Running) {
		Result = OnTick(DeltaTime);
	}
	if (State == Finished) {
		if (Result)
		{
			OnComplete();
		}
		else
		{
			OnFail();
		}
	}
	
	return Result;
}

bool UCustomAction::OnTick(float DeltaTime)
{
	State = Finished;
	return true;
}

void UCustomAction::OnComplete()
{
}

void UCustomAction::OnFail()
{
}
