#pragma once
#include "FIT3094_A1_Code/HLAction.h"
#include "FIT3094_A1_Code/Timer.h"
#include "FIT3094_A1_Code/GridNode.h"
#include "CustomAction.generated.h"

enum ActionState
{
	NotStarted,
	Running,
	Finished,
};

UCLASS()
class UCustomAction : public UHLAction
{
public:
	GENERATED_BODY()
	
	AShip* Agent;
	Timer _Timer;
	ActionState State = NotStarted;

	virtual bool IsActionDone() override;

	virtual bool SetupAction(AShip* Ship) override;

	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;

	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;
	
	virtual bool Execute(AShip* Ship, float DeltaTime) override;
	
	virtual void OnStart();

	/**
	 * @brief would change State to Finished if the action is done
	 * @return false when failed
	 */
	virtual bool OnTick(float DeltaTime);

	virtual void OnComplete();

	virtual void OnFail();

	GridNode* PlannedLocation;

	virtual bool OnActionAborted(AShip* Ship) override;
};
