#pragma once
#include "CustomAction.h"
#include "FIT3094_A1_Code/HLAction.h"

class DepositAction : public CustomAction
{
public:
	virtual bool RequiresInRange() override;

	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;

	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;
	
	virtual bool Execute(AShip* Ship, float DeltaTime) override;
};
