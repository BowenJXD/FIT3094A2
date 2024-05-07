// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollectAction.h"
#include "CollectGrainAction.generated.h"

/**
 * 
 */
UCLASS()
class FIT3094_A1_CODE_API UCollectGrainAction : public UCollectAction
{
	GENERATED_BODY()

public:
	virtual GRID_TYPE GetResourceType() override;
};
