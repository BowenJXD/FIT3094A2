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
	Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, TArray{GRID_TYPE::Home});
	/*if (Target) Agent->LevelGenerator->ResourceOccupancy.Add(Cast<AResource>(Target), Agent);*/
	return Target != nullptr;
}

bool UDepositAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	bool bForward = false;
	bool Result = !bForward;
	if (bForward)
	{
		Result |= CurrentState[AgentWood] > 0;
		Result |= CurrentState[AgentStone] > 0;
		Result |= CurrentState[AgentGrain] > 0;
	}
	else
	{
		Result &= CurrentState[AgentWood] == 0;
		Result &= CurrentState[AgentStone] == 0;
		Result &= CurrentState[AgentGrain] == 0;
	}
	return Result;
}

void UDepositAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	bool bForward = false;

	int Wood = SuccessorState[AgentWood];
	int Stone = SuccessorState[AgentStone];
	int Grain = SuccessorState[AgentGrain];
	if (bForward)
	{
		SuccessorState[AgentWood] -= Wood;
		SuccessorState[AgentStone] -= Stone;
		SuccessorState[AgentGrain] -= Grain;

		SuccessorState[TotalWood] += Wood;
		SuccessorState[TotalStone] += Stone;
		SuccessorState[TotalGrain] += Grain;
	}
	else
	{
		SuccessorState[AgentWood] += Wood;
		SuccessorState[AgentStone] += Stone;
		SuccessorState[AgentGrain] += Grain;

		SuccessorState[TotalWood] -= Wood;
		SuccessorState[TotalStone] -= Stone;
		SuccessorState[TotalGrain] -= Grain;
	}
}

void UDepositAction::OnStart()
{
	float Interval = 1;
	_Timer = Timer(Interval);
}

bool UDepositAction::OnTick(float DeltaTime)
{
	AResource* Resource = Cast<AResource>(Target);
	if (_Timer.Tick(DeltaTime))
	{
		if (!Resource) return false;
		int Deposited = Agent->LevelGenerator->DepositResource(Agent);
		Agent->LevelGenerator->AlterPlannedResources(Agent->GetResourceType(), Deposited);
		State = Finished;
	}
	
	return true;
}

