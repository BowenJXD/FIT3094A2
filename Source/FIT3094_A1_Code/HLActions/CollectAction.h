#pragma once
#include "CustomAction.h"
#include "FIT3094_A1_Code/Resource.h"
#include "FIT3094_A1_Code/Timer.h"
#include "CollectAction.generated.h"

UCLASS()
class FIT3094_A1_CODE_API UCollectAction : public UCustomAction
{	
public:
	GENERATED_BODY()
	
	virtual bool RequiresInRange() override;

	virtual bool SetupAction(AShip* Ship) override;
	
	virtual bool CheckPreconditions(AShip* Ship, TMap<STATE_KEY, int> CurrentState) override;

	virtual void ApplyEffects(AShip* Ship, TMap<STATE_KEY, int>& SuccessorState) override;

	virtual void OnStart() override;
	
	virtual bool OnTick(float DeltaTime) override;

	virtual void OnComplete() override;

	Timer _Timer;
};
