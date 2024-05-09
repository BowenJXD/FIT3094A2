// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GOAPPlanner.h"
#include "HLAction.generated.h"

/**
 * 
 */

class AShip;

UCLASS(Abstract)
class FIT3094_A1_CODE_API UHLAction : public UObject
{
	GENERATED_BODY()
public:
	UHLAction();

	AActor* Target;
	
	bool bInRange;

	float ActionCost;

	//Called by the FSM when executing a planned action
	//Returns true if the action has finished executing, false otherwise
	virtual bool IsActionDone() PURE_VIRTUAL(UGOAPAction::IsActionDone, return false;);

	//This checks to see if the action is valid for the agent @param Ship
	//Returns true if the action is valid for the agent, false otherwise
	virtual bool SetupAction(AShip* Ship) PURE_VIRTUAL(UGOAPAction::SetupAction, return false;);

	//If applied in a forward direction, check to see if preconditions are met in current state
	//If applied in a backward direction, check if the effects are met in the current state
	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) PURE_VIRTUAL(UGOAPAction::CheckPreconditions, return false;);

	//If applied in a forward direction, apply effects to current state
	//If applied in a backward direction, apply preconditions to current state
	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) PURE_VIRTUAL(UGOAPAction::ApplyEffects, return;);

	//This is called by the FSM to execute a planned action
	virtual bool Execute(AShip* Ship, float DeltaTime) PURE_VIRTUAL(UGOAPAction::Tick, return false;);

	//Called by the FSM when executing a planned action
	//Some Actions can only be executing when within Range of a location in the world (GOAPAction::Target)
	//Returns true if ship is at the GOAPAction::Target, false otherwise
	virtual bool RequiresInRange() PURE_VIRTUAL(UGOAPAction::RequiresInRange, return false;);

	// ---------------- New Code ----------------
	virtual void OnActionConfirmed(AShip* Ship) PURE_VIRTUAL(UGOAPAction::OnActionConfirmed, return;);
};

