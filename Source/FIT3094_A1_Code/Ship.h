// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "HLAction.h"
#include "GOAPPlanner.h"
#include "GridNode.h"
#include "GameFramework/Actor.h"
#include "Ship.generated.h"

class ALevelGenerator;

UCLASS()
class FIT3094_A1_CODE_API AShip : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShip();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	enum ACTOR_STATES
	{
		State_Idle,
		State_Move,
		State_Execute
	};

	enum AGENT_TYPE
	{
		Woodcutter,
		Stonemason,
		Farmer,
		Builder
	};
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
		UMaterialInterface* WoodCutterMaterial;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* StoneMasonMaterial;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* FarmerMaterial;
	UPROPERTY(EditAnywhere)
		UMaterialInterface* BuilderMaterial;
	UMaterialInstanceDynamic* MaterialInstance;
	float CrashTime = 0;
	FLinearColor OldColour;
	TArray<UActorComponent*> Meshes;

	UPROPERTY(EditAnywhere)
		float MoveSpeed;
	UPROPERTY(EditAnywhere)
		float Tolerance;
	
	TArray<GridNode*> Path;
	UPROPERTY()
		TArray<AActor*> PathDisplayActors;
	GridNode* GoalNode;
	GridNode* CurrentNode;
	GridNode* LastNode;
	ALevelGenerator* LevelGenerator;
	FVector Direction;
	bool bRecentlyCrashed = false;
	bool bAtNextNode = false;
	bool bAtGoal = false;
	

	ACTOR_STATES CurrentState;
	
	void OnIdleEnter();
	void OnIdleTick(float DeltaTime);
	void OnIdleExit();

	void OnMoveEnter();
	void OnMoveTick(float DeltaTime);
	void OnMoveExit();

	void OnExecuteEnter();
	void OnExecuteTick(float DeltaTime);
	void OnExecuteExit();

	void ChangeState(ACTOR_STATES NextState);

	float MaxIdleTime = 0.5;
	int FailedPlanCooldown = 0;

	UPROPERTY()
		TArray<TSubclassOf<UHLAction>> AvailableActions;
	UPROPERTY()
		TArray<UHLAction*> PlannedActions;
	AGENT_TYPE AgentType;

	UPROPERTY(VisibleAnywhere)
		int NumWood;
	UPROPERTY(VisibleAnywhere)
		int NumStone;
	UPROPERTY(VisibleAnywhere)
		int NumGrain;

	TMap<STATE_KEY, int> GetWorldState();

	// ----------------- YOUR CODE -----------------
	
	TArray<TTuple<STATE_KEY, int, char>> PickGoal();
	void OnPlanFailed(TMap<STATE_KEY, int> FailedGoalState);
	/**
	 * @brief 
	 * @param FailedAction 
	 * @return Whether to continue or not
	 */
	bool OnPlanAborted(UHLAction* FailedAction);
	void AddActions();
	bool FSMPlan();
	
	//CHANGE THIS TO TRUE IN A2B
	UPROPERTY(EditAnywhere)
		bool bUseGOAP = false;

	// ----------------- NEW CODE -----------------

	GRID_TYPE GetResourceType();

	float GetTravelTime(int PathCount);

	// Collect n Destroy the resource point and change to shallow water
};
