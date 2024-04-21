#include "CollectAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"

bool UCollectAction::RequiresInRange()
{
	return true;
}

bool UCollectAction::SetupAction(AShip* Ship)
{
	Super::SetupAction(Ship);
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
		Interval = 2;
		if (Executor->AgentType == AShip::AGENT_TYPE::Woodcutter)
		{
			Interval /= 3;
		}
		break;
	case GRID_TYPE::Stone:
		Interval = 3;
		if (Executor->AgentType == AShip::AGENT_TYPE::Stonemason)
		{
			Interval /= 3;
		}
		break;
	case GRID_TYPE::Grain:
		Interval = 5;
		if (Executor->AgentType == AShip::AGENT_TYPE::Farmer)
		{
			Interval /= 3;
		}
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
	AResource* Resource = Cast<AResource>(Target);
	if (!Resource) return false;

	if ( _Timer.Tick(DeltaTime))
	{
		if (!Executor->LevelGenerator->CollectResource(Executor, Resource))
		{
			State = Finished;
		}
	}
	
	return true;
}