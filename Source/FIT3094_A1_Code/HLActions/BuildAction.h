// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "CustomAction.h"
#include "FIT3094_A1_Code/HLAction.h"
#include "FIT3094_A1_Code/Timer.h"
#include "BuildAction.generated.h"

/**
 * 
 */
UCLASS()
class FIT3094_A1_CODE_API UBuildAction : public UCustomAction
{
	GENERATED_BODY()

public:
	BUILDING_TYPE BuildingType = BUILDING_TYPE::University;
	
	TSubclassOf<ABuilding> BuildingBlueprint;

	virtual bool RequiresInRange() override;

	virtual bool SetupAction(AShip* Ship) override;

	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;

	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;
	
	virtual void OnStart() override;

	virtual bool OnTick(float DeltaTime) override;

	virtual void OnComplete() override;

	virtual float OnActionConfirmed(AShip* Ship, float PlanningTime) override;

	virtual BUILDING_TYPE GetBuildingType() PURE_VIRTUAL(UBuildAction::GetBuildingType, return BUILDING_TYPE::University;);
};
