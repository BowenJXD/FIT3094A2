#pragma once
#include "FIT3094_A1_Code/HLAction.h"
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
	
	AShip* Executor;
	ActionState State = NotStarted;

	virtual bool IsActionDone() override;

	virtual bool SetupAction(AShip* Ship) override;
	
	virtual bool Execute(AShip* Ship, float DeltaTime) override;
	
	virtual void OnStart() PURE_VIRTUAL(CustomAction::OnStart);

	/**
	 * @brief would change State to Finished if the action is done
	 * @return false when failed
	 */
	virtual bool OnTick(float DeltaTime);

	virtual void OnComplete();

	virtual void OnFail();
};
