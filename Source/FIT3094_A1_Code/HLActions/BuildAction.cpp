// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"
#include "FIT3094_A1_Code/Util/StatisticExporter.h"

bool UBuildAction::RequiresInRange()
{
	return true;
}

bool UBuildAction::SetupAction(AShip* Ship)
{
	Super::SetupAction(Ship);
	Target = Ship->LevelGenerator->CalculateNearestGoal(PlannedLocation, TArray{GRID_TYPE::BuildingSlot}, ABuilding::GetTimeRequired(GetBuildingType()), Target);
	return Target != nullptr;
}

bool UBuildAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	Super::CheckPreconditions(Ship, CurrentState);
	
	bool Result = Ship->AgentType == AShip::AGENT_TYPE::Builder
	&& CurrentState[AgentWood] == 0
	&& CurrentState[AgentStone] == 0
	&& CurrentState[AgentGrain] == 0
	&& CurrentState[NumBuildingSlots] > 0
	&& CurrentState[TimeLeft] > ABuilding::GetTimeRequired(GetBuildingType()) + 2 // for moving time
	&& CurrentState[TotalWood] >= ABuilding::GetResourceCost(GetBuildingType())[0]
	&& CurrentState[TotalStone] >= ABuilding::GetResourceCost(GetBuildingType())[1]
	&& CurrentState[TotalGrain] >= ABuilding::GetResourceCost(GetBuildingType())[2];
	return Result;
}

void UBuildAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	Super::ApplyEffects(Ship, SuccessorState);
	
	SuccessorState[TotalWood] -= ABuilding::GetResourceCost(GetBuildingType())[0];
	SuccessorState[TotalStone] -= ABuilding::GetResourceCost(GetBuildingType())[1];
	SuccessorState[TotalGrain] -= ABuilding::GetResourceCost(GetBuildingType())[2];
	SuccessorState[NumBuildings]++;
	SuccessorState[NumBuildingSlots]--;
	SuccessorState[NumPoints] += ABuilding::GetPointsProvided(GetBuildingType());
}

void UBuildAction::OnStart()
{	
	auto Node = Cast<AResource>(Target);
	_Timer = Timer(ABuilding::GetTimeRequired(GetBuildingType()));
}

bool UBuildAction::OnTick(float DeltaTime)
{
	AResource* Resource = Cast<AResource>(Target);
	if (_Timer.Tick(DeltaTime))
	{
		auto Building = GetWorld()->SpawnActor<ABuilding>(Agent->LevelGenerator->BuildingBlueprint,
														  Target->GetActorLocation(), FRotator::ZeroRotator);
		Building->BuildingType = GetBuildingType();
		Agent->LevelGenerator->Points += Building->GetPointsProvided();
		StatisticsExporter::Get().University++;
		
		UE_LOG(LogTemp, Warning, TEXT("%s of type %s Deleted!"), *Resource->GetName(), *Resource->GetResourceType());
		Target->Destroy();
		Agent->LevelGenerator->FindGridNode(Resource)->GridType = GRID_TYPE::ShallowWater;
		
		State = Finished;
	}

	return true;
}

void UBuildAction::OnComplete()
{
	
}

float UBuildAction::OnActionConfirmed(AShip* Ship, float PlanningTime)
{
	Agent->LevelGenerator->TotalWood -= ABuilding::GetResourceCost(GetBuildingType())[0];
	Agent->LevelGenerator->TotalStone -= ABuilding::GetResourceCost(GetBuildingType())[1];
	Agent->LevelGenerator->TotalGrain -= ABuilding::GetResourceCost(GetBuildingType())[2];

	if (Target)
	{
		auto oc = Agent->LevelGenerator->AddOccupancy(Cast<AResource>(Target), Agent, PlannedLocation, PlanningTime, ABuilding::GetTimeRequired(GetBuildingType()));
		return oc.EndTime;
	}
	return 0;
}
