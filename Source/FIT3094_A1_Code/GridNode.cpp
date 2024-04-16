// Fill out your copyright notice in the Description page of Project Settings.


#include "GridNode.h"


GridNode::GridNode()
{
	X = 0;
	Y = 0;
	G = 0;
	H = 0;
	F = 0;

	GridType = GRID_TYPE::DeepWater;
	Parent = nullptr;
	ResourceAtLocation = nullptr;
	BuildingAtLocation = nullptr;
}

bool GridNode::operator>(const GridNode&& other) const
{
	if(F > other.F)
	{
		return true;
	}
	if (F == other.F)
	{
		if(G <= other.G)
		{
			return true;
		}
	}
	return false;
}

float GridNode::GetTravelCost() const
{
	switch(GridType)
	{
	case GRID_TYPE::Land:
		return 100;
	case GRID_TYPE::DeepWater:
		return 3;
	default:
		return 1;
	}
}
