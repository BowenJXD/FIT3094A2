// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "GridNode.h"
#include "Resource.h"
#include "Ship.h"
#include "GameFramework/Actor.h"
#include "HLAction.h"
#include "LevelGenerator.generated.h"

using namespace std;

DECLARE_LOG_CATEGORY_EXTERN(Collisions, Warning, All);

UCLASS()
class FIT3094_A1_CODE_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

public:
	
	// Sets default values for this actor's properties
	ALevelGenerator();

	struct CompareGridNodes
	{
		bool operator()(const GridNode* a, const GridNode* b) const
		{
			if(a->F > b->F)
			{
				return true;
			}
			if(a->F == b->F)
			{
				if(a->G <= b->G)
				{
					return true;
				}
			}
			return false;
		}
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	static const int MAX_MAP_SIZE = 200;
	static const int GRID_SIZE_WORLD = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MapSizeX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MapSizeY;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UHLAction>> ActionClasses;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DeepBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> LandBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ShallowBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> WoodBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> StoneBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> GrainBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AResource> BuildLocationBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> BuildingBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ShipBlueprint;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> PathDisplayBlueprint;
	UPROPERTY()
	TArray<AActor*> PathDisplayActors;
	UPROPERTY(EditAnywhere)
	AActor* Camera;

	bool bCameraRotated = false;
	
	GridNode* WorldArray[MAX_MAP_SIZE][MAX_MAP_SIZE];
	TArray<AActor*> Terrain;
	TArray<FVector2d> ShipSpawns;
	TArray<FVector2d> ResourceSpawns;
	TArray<AShip*> Ships;
	
	int TotalIndex = 50;
	//Town Centres/Wood/Stone/Grain/Taverns/BuildingSlots
	int ResourcesToSpawn [6] = {10, 120, 120, 120, 60};
	//Woodcutters/Stonemasons/Farmers/Builders
	int ShipsToSpawn [4] = {5, 5, 5, 3};
	
	int TotalWood = 0;
	int TotalStone = 0;
	int TotalGrain = 0;
	int Points = 0;
	int TimeLimit = 180;
	int StartTime;

	void SpawnWorldActors(TArray<TArray<char>> Grid);
	void GenerateNodeGrid(TArray<TArray<char>> Grid);
	void ResetAllNodes();
	int CalculateManhattanDistanceBetween(GridNode* First, GridNode* Second);
	void GenerateWorldFromFile(TArray<FString> WorldArrayStrings);
	void GenerateScenarioFromFile(TArray<FString> ScenarioArrayStrings);
	void InitialisePaths();
	void RenderPath(AShip* Ship);
	void ResetPath();
	void InitialiseLevel();
	void DestroyAllActors();
	void CalculatePath(AShip* Ship, GridNode* Resource);
	TArray<GridNode*> GetNeighbours(GridNode* NodeToCheck);
	void CheckForCollisions();
	AActor* CalculateNearestGoal(AActor* Ship, GRID_TYPE ResourceType);
	GridNode* FindGridNode(AActor* ActorResource);
	void Replan(AShip* Ship);

	//CHANGE THIS TO TRUE TO ENABLE REPLANNING
	bool CollisionAndReplanning = false;
	
	int NumReplans = 0;
	int NumCollisions = 0;
	
	// ----------------- NEW CODE -----------------
	bool CollectResource(AShip* Ship, AResource* Resource);
	
};
