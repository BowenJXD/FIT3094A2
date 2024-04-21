#include "DepositAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"

bool UDepositAction::RequiresInRange()
{
	return true;
}

bool UDepositAction::SetupAction(AShip* Ship)
{
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
	auto Node = Cast<AResource>(Target);
	if (Node->ResourceType == GRID_TYPE::Home)
	{
		float Amount = Executor->NumWood;
		Executor->NumWood = 0;
		Executor->LevelGenerator->TotalWood += Amount;

		float Amount2 = Executor->NumStone;
		Executor->NumStone = 0;
		Executor->LevelGenerator->TotalStone += Amount2;

		float Amount3 = Executor->NumGrain;
		Executor->NumGrain = 0;
		Executor->LevelGenerator->TotalGrain += Amount3;
	}
}

bool UDepositAction::OnTick(float DeltaTime)
{
	return Super::OnTick(DeltaTime);
}

