// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildAction.h"
#include "BuildTheatreAction.generated.h"

/**
 * 
 */
UCLASS()
class FIT3094_A1_CODE_API UBuildTheatreAction : public UBuildAction
{
	GENERATED_BODY()

public:
	virtual BUILDING_TYPE GetBuildingType() override;
};
