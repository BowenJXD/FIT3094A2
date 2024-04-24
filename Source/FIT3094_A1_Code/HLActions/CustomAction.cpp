#include "CustomAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"

bool UCustomAction::IsActionDone()
{
	return State == Finished;
}

bool UCustomAction::SetupAction(AShip* Ship)
{
	Agent = Ship;
	return true;
}

void UCustomAction::OnStart()
{
}

bool UCustomAction::Execute(AShip* Ship, float DeltaTime)
{
	bool Result = false;
	if (RequiresInRange() && !Target || !IsValid(Target))
	{
		State = Finished;
		UE_LOG(LogTemp, Warning, TEXT("Target %s is invalid!"), *Target->GetName());
		return false;
	}
	
	if (State == NotStarted) {
		OnStart();
		State = Running;
	}
	if (State == Running) {
		Result = OnTick(DeltaTime);
	}
	if (State == Finished) {
		if (RequiresInRange()) Agent->LevelGenerator->ResourceOccupancy.Remove(Cast<AResource>(Target));
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
