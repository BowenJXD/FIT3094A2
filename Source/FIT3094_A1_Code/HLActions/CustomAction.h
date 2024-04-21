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
	
	virtual bool Execute(AShip* Ship, float DeltaTime) override;
	
	virtual void OnStart() PURE_VIRTUAL(CustomAction::OnStart);

	/**
	 * @brief would change State to Finished if the action is done
	 * @return 
	 */
	virtual bool OnTick(float DeltaTime) PURE_VIRTUAL(CustomAction::OnTick, {
		State = Finished;
		return true;
	});

	virtual void OnComplete() PURE_VIRTUAL(CustomAction::OnComplete);

	virtual void OnFail() PURE_VIRTUAL(CustomAction::OnFail);
};
