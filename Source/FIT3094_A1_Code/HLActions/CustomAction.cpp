﻿#include "CustomAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"
#include "FIT3094_A1_Code/Util/StatisticExporter.h"

bool UCustomAction::IsActionDone()
{
	return State == Finished;
}

bool UCustomAction::SetupAction(AShip* Ship)
{
	Agent = Ship;
	return true;
}

bool UCustomAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	PlannedLocation = Ship->LevelGenerator->WorldArray[CurrentState[AgentLocationY]][CurrentState[AgentLocationX]];
	return true;
}

void UCustomAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	GridNode* TargetLocation = Ship->LevelGenerator->GetNode(Target);
	SuccessorState[AgentLocationX] = TargetLocation->X;
	SuccessorState[AgentLocationY] = TargetLocation->Y;
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

	FString Key = GetClass()->GetName();
	if (!StatisticsExporter::Get().ExecuteTimes.Contains(Key))
	{
		StatisticsExporter::Get().ExecuteTimes.Add(Key, 0);
	}
	StatisticsExporter::Get().ExecuteTimes[Key] += DeltaTime;
	
	if (State == NotStarted) {
		OnStart();
		State = Running;
	}
	if (State == Running) {
		Result = OnTick(DeltaTime);
	}
	if (State == Finished) {
		if (RequiresInRange() && Agent->LevelGenerator->ResourceOccupancy.Contains(Cast<AResource>(Target)))
		{
			Occupancy* oc = Agent->LevelGenerator->ResourceOccupancy.Find(Cast<AResource>(Target));
			float End = oc->EndTime;
			float Now = Agent->LevelGenerator->TimePassed;
			float Diff = (Now - End) / (oc->EndTime - oc->StartTime);
			UE_LOG(LogTemp, Warning,
			       TEXT("Action %s to finish at %f but actually finished at %f, having a diff of %f (%f)"),
			       *GetName(), End, Now, Now - End, Diff);
			Agent->LevelGenerator->ResourceOccupancy.Remove(Cast<AResource>(Target));
		}
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

bool UCustomAction::OnActionAborted(AShip* Ship)
{
	bool Result = SetupAction(Ship);
	return Result;
}
