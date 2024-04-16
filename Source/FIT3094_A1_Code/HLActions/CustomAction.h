#pragma once
#include "FIT3094_A1_Code/HLAction.h"

enum ActionState
{
	NotStarted,
	Running,
	Finished,
};

class CustomAction : public UHLAction
{
public:

	AShip* Executor;
	ActionState State = NotStarted;

	virtual bool IsActionDone() override;
	
	virtual bool Execute(AShip* Ship, float DeltaTime) override;
	
	virtual void OnStart() PURE_VIRTUAL(CustomAction::OnStart);

	/**
	 * @brief would change State to Finished if the action is done
	 * @return 
	 */
	 virtual bool OnTick(float DeltaTime) PURE_VIRTUAL(CustomAction::OnTick, return false;);

	virtual void OnFinish() PURE_VIRTUAL(CustomAction::OnFinish);
};
