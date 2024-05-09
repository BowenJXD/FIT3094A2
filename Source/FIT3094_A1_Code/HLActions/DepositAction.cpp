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
	Target = Ship->LevelGenerator->CalculateNearestGoal(PlannedLocation, TArray{GRID_TYPE::Home}, 1);
	return Target != nullptr;
}

bool UDepositAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	Super::CheckPreconditions(Ship, CurrentState);
	
	bool Result = false;
	Result |= CurrentState[AgentWood] > 0;
	Result |= CurrentState[AgentStone] > 0;
	Result |= CurrentState[AgentGrain] > 0;
	return Result;
}

void UDepositAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	Super::ApplyEffects(Ship, SuccessorState);
	
	int Wood = SuccessorState[AgentWood];
	int Stone = SuccessorState[AgentStone];
	int Grain = SuccessorState[AgentGrain];
	
	SuccessorState[AgentWood] -= Wood;
	SuccessorState[AgentStone] -= Stone;
	SuccessorState[AgentGrain] -= Grain;

	SuccessorState[TotalWood] += Wood;
	SuccessorState[TotalStone] += Stone;
	SuccessorState[TotalGrain] += Grain;
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

void UDepositAction::OnActionConfirmed(AShip* Ship)
{
	if (Target) Agent->LevelGenerator->AddOccupancy(Cast<AResource>(Target), Agent, Agent->Path.Num(), 1);
}

