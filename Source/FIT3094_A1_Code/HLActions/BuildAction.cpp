// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildAction.h"

#include "FIT3094_A1_Code/LevelGenerator.h"
#include "FIT3094_A1_Code/Ship.h"

bool UBuildAction::RequiresInRange()
{
	return true;
}

bool UBuildAction::SetupAction(AShip* Ship)
{
	Super::SetupAction(Ship);
	Target = Ship->LevelGenerator->CalculateNearestGoal(Ship, GRID_TYPE::BuildingSlot);
	if (!Target) return false;
	Executor->LevelGenerator->TotalWood -= ABuilding::GetResourceCost(BuildingType)[0];
	Executor->LevelGenerator->TotalStone -= ABuilding::GetResourceCost(BuildingType)[1];
	Executor->LevelGenerator->TotalGrain -= ABuilding::GetResourceCost(BuildingType)[2];
	return Target != nullptr;
}

bool UBuildAction::CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState)
{
	bool Result = Ship->AgentType == AShip::AGENT_TYPE::Builder
	&& CurrentState[AgentWood] == 0
	&& CurrentState[AgentStone] == 0
	&& CurrentState[AgentGrain] == 0
	&& CurrentState[TotalWood] > ABuilding::GetResourceCost(BuildingType)[0]
	&& CurrentState[TotalStone] > ABuilding::GetResourceCost(BuildingType)[1]
	&& CurrentState[TotalGrain] > ABuilding::GetResourceCost(BuildingType)[2];
	return Result;
}

void UBuildAction::ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState)
{
	SuccessorState[TotalWood] -= ABuilding::GetResourceCost(BuildingType)[0];
	SuccessorState[TotalStone] -= ABuilding::GetResourceCost(BuildingType)[1];
	SuccessorState[TotalGrain] -= ABuilding::GetResourceCost(BuildingType)[2];
}

void UBuildAction::OnStart()
{	
	auto Node = Cast<AResource>(Target);
	_Timer = Timer(ABuilding::GetTimeRequired(BuildingType));
}

bool UBuildAction::OnTick(float DeltaTime)
{
	if (!Target || !IsValid(Target))
	{
		State = Finished;
		return false;
	}
	AResource* Resource = Cast<AResource>(Target);
	if (_Timer.Tick(DeltaTime))
	{
		auto Building = GetWorld()->SpawnActor<ABuilding>(Executor->LevelGenerator->BuildingBlueprint,
														  Target->GetActorLocation(), FRotator::ZeroRotator);
		Building->BuildingType = BuildingType;
		Executor->LevelGenerator->Points += Building->GetPointsProvided();
		
		Executor->LevelGenerator->FindGridNode(Resource)->GridType = GRID_TYPE::ShallowWater;
		Target->Destroy();
		State = Finished;
	}

	return true;
}

void UBuildAction::OnComplete()
{
	
}
