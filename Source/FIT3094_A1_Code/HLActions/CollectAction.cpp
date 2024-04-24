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
	ALevelGenerator* LG = Ship->LevelGenerator;
	float ShipTypeWeight = 1.0f;
	GRID_TYPE ShipType = Ship->GetResourceType();
	TArray<GRID_TYPE> Types;
	/*float WoodRequired = (LG->TotalWood + PlannedResourceAmount - 15.0f) / 15.0f - (ShipType == GRID_TYPE::Wood) * ShipTypeWeight;
	float StoneRequired = (LG->TotalStone + PlannedResourceAmount - 10.0f) / 10.0f - (ShipType == GRID_TYPE::Stone) * ShipTypeWeight;
	float GrainRequired = (LG->TotalGrain + PlannedResourceAmount - 5.0f) / 5.0f - (ShipType == GRID_TYPE::Grain) * ShipTypeWeight;*/
	float WoodRequired = (LG->TotalWood - 15.0f - (ShipType == GRID_TYPE::Wood) * ShipTypeWeight) / 15.0;
	float StoneRequired = (LG->TotalStone - 10.0f - (ShipType == GRID_TYPE::Stone) * ShipTypeWeight) / 10.0f;
	float GrainRequired = (LG->TotalGrain - 5.0f - (ShipType == GRID_TYPE::Grain) * ShipTypeWeight) / 5.0f;
	float Min = FMath::Min(TArray{WoodRequired, StoneRequired, GrainRequired});
	if (Min == WoodRequired) Types.Add(GRID_TYPE::Wood);
	if (Min == StoneRequired) Types.Add(GRID_TYPE::Stone);
	if (Min == GrainRequired) Types.Add(GRID_TYPE::Grain);

	TArray<GRID_TYPE> ResultTypes = TArray{ShipType};
	if (!Types.Contains(ShipType)) ResultTypes = Types; 
	
	Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, ResultTypes);
	if (!Target) return false;
	PlannedResourceAmount = Cast<AResource>(Target)->ResourceCount;
	LG->AlterPlannedResources(ShipType, PlannedResourceAmount);

	if (Target) Agent->LevelGenerator->ResourceOccupancy.Add(Cast<AResource>(Target), Agent);
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
	switch (Agent->GetResourceType())
	{
	case GRID_TYPE::Wood:
		Interval = 2;
		if (Agent->AgentType == AShip::AGENT_TYPE::Woodcutter)
		{
			Interval /= 1.5;
		}
		break;
	case GRID_TYPE::Stone:
		Interval = 3;
		if (Agent->AgentType == AShip::AGENT_TYPE::Stonemason)
		{
			Interval /= 1.5;
		}
		break;
	case GRID_TYPE::Grain:
		Interval = 5;
		if (Agent->AgentType == AShip::AGENT_TYPE::Farmer)
		{
			Interval /= 1.5;
		}
		break;
	default:
		Interval = 0;
		break;
	}
	
	_Timer = Timer(Interval);
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