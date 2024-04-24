#include "DepositAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"

bool UDepositAction::RequiresInRange()
{
	return true;
}

bool UDepositAction::SetupAction(AShip* Ship)
{
	Super::SetupAction(Ship);
	Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::Home);
	return Target != nullptr;
}

bool UDepositAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	bool Result = false;
	Result |= CurrentState[AgentWood] > 0;
	Result |= CurrentState[AgentStone] > 0;
	Result |= CurrentState[AgentGrain] > 0;
	return Result;
}

void UDepositAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	SuccessorState.Add(AgentWood, 0);
	SuccessorState.Add(AgentStone, 0);
	SuccessorState.Add(AgentGrain, 0);
}

void UDepositAction::OnStart()
{
	float Interval = 1;
	_Timer = Timer(Interval);
}

bool UDepositAction::OnTick(float DeltaTime)
{
	if (!Target || !IsValid(Target))
	{
		State = Finished;
		return false;
	}
	AResource* Resource = Cast<AResource>(Target);
	if (_Timer.Tick(DeltaTime))
	{
		if (!Resource) return false;
		int Deposited = Executor->LevelGenerator->DepositResource(Executor);
		Executor->LevelGenerator->AlterPlannedResources(Executor->GetResourceType(), Deposited);
		State = Finished;
	}
	
	return true;
}

