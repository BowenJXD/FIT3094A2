#include "DepositAction.h"

bool DepositAction::RequiresInRange()
{
	return true;
}

bool DepositAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	bool Result = false;
	Result |= CurrentState[AgentWood] > 0;
	Result |= CurrentState[AgentStone] > 0;
	Result |= CurrentState[AgentGrain] > 0;
	return Result;
}

void DepositAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	SuccessorState.Add(AgentWood, 0);
	SuccessorState.Add(AgentStone, 0);
	SuccessorState.Add(AgentGrain, 0);
}

bool DepositAction::Execute(AShip* Ship, float DeltaTime)
{
	
	return Target;
}
