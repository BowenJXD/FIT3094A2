// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FIT3094_A1_Code/HLAction.h"
#include "FindResourceAction.generated.h"

/**
 * 
 */
UCLASS()
class FIT3094_A1_CODE_API UFindResourceAction : public UHLAction
{
	GENERATED_BODY()

	virtual bool RequiresInRange() override;

	
};
