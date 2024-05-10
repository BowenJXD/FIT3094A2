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
	//Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, TArray{GRID_TYPE::Wood, GRID_TYPE::Stone, GRID_TYPE::Grain});

	// find the most needed resource to build a university
	/*ALevelGenerator* LG = Ship->LevelGenerator;
	float ShipTypeWeight = 5.0f;
	GRID_TYPE ShipType = Ship->GetResourceType();
	TArray<GRID_TYPE> Types;
	float WoodRequired = (LG->TotalWood + PlannedResourceAmount - 15.0f) / 15.0f - (ShipType == GRID_TYPE::Wood) * ShipTypeWeight;
	float StoneRequired = (LG->TotalStone + PlannedResourceAmount - 10.0f) / 10.0f - (ShipType == GRID_TYPE::Stone) * ShipTypeWeight;
	float GrainRequired = (LG->TotalGrain + PlannedResourceAmount - 5.0f) / 5.0f - (ShipType == GRID_TYPE::Grain) * ShipTypeWeight;
	/*float WoodRequired = (LG->TotalWood - 15.0f - (ShipType == GRID_TYPE::Wood) * ShipTypeWeight) / 15.0;
	float StoneRequired = (LG->TotalStone - 10.0f - (ShipType == GRID_TYPE::Stone) * ShipTypeWeight) / 10.0f;
	float GrainRequired = (LG->TotalGrain - 5.0f - (ShipType == GRID_TYPE::Grain) * ShipTypeWeight) / 5.0f;#1#
	float Min = FMath::Min(TArray{WoodRequired, StoneRequired, GrainRequired});
	if (Min == WoodRequired) Types.Add(GRID_TYPE::Wood);
	if (Min == StoneRequired) Types.Add(GRID_TYPE::Stone);
	if (Min == GrainRequired) Types.Add(GRID_TYPE::Grain);

	TArray<GRID_TYPE> ResultTypes = TArray{ShipType};
	if (!Types.Contains(ShipType)) ResultTypes = Types; 
	
	Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, ResultTypes);*/
	Target = Ship->LevelGenerator->CalculateNearestGoal(PlannedLocation, TArray{GetResourceType()}, 5);
	if (!Target) return false;
	return Target != nullptr;
}

bool UCollectAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
 	Super::CheckPreconditions(Ship, CurrentState);
	
	bool Result = true;
	Result &= CurrentState[AgentWood] == 0;
	Result &= CurrentState[AgentStone] == 0;
	Result &= CurrentState[AgentGrain] == 0;
	return Result;
}

void UCollectAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	Super::ApplyEffects(Ship, SuccessorState);
	
	AResource* Resource = Cast<AResource>(Target);
	if (Resource->ResourceType == GRID_TYPE::Wood)
	{
		SuccessorState[AgentWood] ++;
		if (Agent->AgentType == AShip::AGENT_TYPE::Woodcutter)
		{
			SuccessorState[AgentWood] ++;
		}
	}
	else if (Resource->ResourceType == GRID_TYPE::Stone)
	{
		SuccessorState[AgentStone] ++;
		if (Agent->AgentType == AShip::AGENT_TYPE::Stonemason)
		{
			SuccessorState[AgentStone] ++;
		}
	}
	else if (Resource->ResourceType == GRID_TYPE::Grain)
	{
		SuccessorState[AgentGrain] ++;
		if (Agent->AgentType == AShip::AGENT_TYPE::Farmer)
		{
			SuccessorState[AgentGrain] ++;
		}
	}	
}


void UCollectAction::OnStart()
{
	
}

bool UCollectAction::OnTick(float DeltaTime)
{
	AResource* Resource = Cast<AResource>(Target);
	if (_Timer.Tick(DeltaTime))
	{
		if (!Resource) return false;
		int Collected = Agent->LevelGenerator->CollectResource(Agent, Resource);
		Agent->LevelGenerator->AlterPlannedResources(Agent->GetResourceType(), -PlannedResourceAmount + Collected);
		State = Finished;
	}
	
	return true;
}

GRID_TYPE UCollectAction::GetResourceType()
{
	return Cast<AResource>(Target)->ResourceType;
}

float UCollectAction::OnActionConfirmed(AShip* Ship, float PlanningTime)
{
	PlannedResourceAmount = Cast<AResource>(Target)->ResourceCount;
	Ship->LevelGenerator->AlterPlannedResources(Ship->GetResourceType(), PlannedResourceAmount);

	switch (Agent->GetResourceType())
	{
	case GRID_TYPE::Wood:
		Duration = 2;
		if (Agent->AgentType == AShip::AGENT_TYPE::Woodcutter)
		{
			Duration /= 1.5;
		}
		break;
	case GRID_TYPE::Stone:
		Duration = 3;
		if (Agent->AgentType == AShip::AGENT_TYPE::Stonemason)
		{
			Duration /= 1.5;
		}
		break;
	case GRID_TYPE::Grain:
		Duration = 5;
		if (Agent->AgentType == AShip::AGENT_TYPE::Farmer)
		{
			Duration /= 1.5;
		}
		break;
	default:
		Duration = 0;
		break;
	}
	
	_Timer = Timer(Duration);

	if (Target)
    {
    	auto oc = Agent->LevelGenerator->AddOccupancy(Cast<AResource>(Target), Agent, PlannedLocation, PlanningTime, 5);
    	return oc.EndTime;
    }
    return 0;
}
