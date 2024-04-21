#include "CollectAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"

bool UCollectAction::RequiresInRange()
{
	return true;
}

bool UCollectAction::SetupAction(AShip* Ship)
{
	Executor = Ship;
	Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, Ship->GetResourceType());
	return Target != nullptr;
}

bool UCollectAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	bool Result = true;
	Result &= CurrentState[AgentWood] == 0;
	Result &= CurrentState[AgentStone] == 0;
	Result &= CurrentState[AgentGrain] == 0;
	return Result;
}

void UCollectAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	
}

void UCollectAction::OnStart()
{
	float Interval;
	float Loop = Cast<AResource>(Target)->ResourceCount;
	switch (Executor->GetResourceType())
	{
	case GRID_TYPE::Wood:
		Interval = 0.1f;
		break;
	case GRID_TYPE::Stone:
		Interval = 3.0f;
		break;
	case GRID_TYPE::Grain:
		Interval = 5.0f;
		break;
	default:
		Interval = 0;
		break;
	}
	
	_Timer = Timer(Interval, Loop);
}

bool UCollectAction::OnTick(float DeltaTime)
{
	/*if (!Target)
	{
		Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, Ship->GetResourceType());
		return false;
	}*/
	bool Result = true;
	AResource* Resource = Cast<AResource>(Target);
	if (!Resource) return false;

	if ( _Timer.Tick(DeltaTime))
	{
		Result = Executor->LevelGenerator->CollectResource(Executor, Resource);
	}
	
	return Result;
}

void UCollectAction::OnComplete()
{
	
}