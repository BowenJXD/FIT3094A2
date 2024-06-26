// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * 
 */

UENUM()
enum class GRID_TYPE : uint8
{
	DeepWater,
	Land,
	ShallowWater,
	Home,
	Treasure,
	Wood,
	Stone,
	Grain,
	BuildingSlot
};

class FIT3094_A1_CODE_API GridNode
{

public:

	GridNode();

	int X;
	int Y;
	int G;
	float H;
	float F;

	GRID_TYPE GridType;
	GridNode* Parent;
	bool bIsInClosed = false;
	bool bIsInOpen = false;
	AActor* ResourceAtLocation;
	AActor* BuildingAtLocation;

	bool operator>(const GridNode&& other) const;
	
	float GetTravelCost() const;
};
