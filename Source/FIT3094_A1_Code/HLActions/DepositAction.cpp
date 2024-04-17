#include "DepositAction.h"

bool UDepositAction::RequiresInRange()
{
	return true;
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

bool UDepositAction::Execute(AShip* Ship, float DeltaTime)
{
	
	return Target == nullptr;
}
