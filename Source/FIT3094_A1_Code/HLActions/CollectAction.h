#pragma once
#include "CustomAction.h"
#include "FIT3094_A1_Code/Util/Timer.h"

class FIT3094_A1_CODE_API CollectAction : public CustomAction
{	
public:
	virtual bool RequiresInRange() override;
	
	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;

	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;

	virtual void OnStart() override;
	
	virtual bool OnTick(float DeltaTime) override;

	virtual void OnFinish() override;

	Timer _Timer;
};
