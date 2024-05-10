// Fill out your copyright notice in the Description page of Project Settings.
#include "Ship.h"

#include "GOAPPlanner.h"
#include "LevelGenerator.h"
#include "HLActions/BuildAction.h"
#include "HLActions/BuildMarketAction.h"
#include "HLActions/BuildTheatreAction.h"
#include "HLActions/BuildUniversityAction.h"
#include "HLActions/CollectAction.h"
#include "HLActions/CollectGrainAction.h"
#include "HLActions/CollectStoneAction.h"
#include "HLActions/CollectWoodAction.h"
#include "HLActions/DepositAction.h"
#include "Kismet/GameplayStatics.h"
#include "Util/StatisticExporter.h"

// Sets default values
AShip::AShip()
{
 	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	MoveSpeed = 500;
	Tolerance = MoveSpeed / 100;
	GoalNode = nullptr;
}

// Called when the game starts or when spawned
void AShip::BeginPlay()
{
	Super::BeginPlay();
	LevelGenerator = Cast<ALevelGenerator>(UGameplayStatics::GetActorOfClass(GetWorld(), ALevelGenerator::StaticClass()));
	GetComponents(UStaticMeshComponent::StaticClass(), Meshes);

	CurrentState = State_Idle;
	AddActions();
	
}

// Called every frame
void AShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(CrashTime > 0)
	{
		CrashTime -= DeltaTime;
		if(CrashTime < 0)
		{
			MaterialInstance->SetVectorParameterValue("Colour", OldColour);
		}
		else
		{
			MaterialInstance->SetVectorParameterValue("Colour", FLinearColor::Red);
		}
	}
	if(NumWood > 0 || NumStone > 0 || NumGrain > 0)
	{
		MaterialInstance->SetScalarParameterValue("Emissive", 2.5);
	}
	else
	{
		MaterialInstance->SetScalarParameterValue("Emissive", 0);
	}

	switch(CurrentState)
	{
	case State_Idle:
		OnIdleTick(DeltaTime);
		break;
	case State_Move:
		OnMoveTick(DeltaTime);
		break;
	case State_Execute:
		OnExecuteTick(DeltaTime);
		break;
	}
	
}

void AShip::OnIdleEnter()
{
		
}

void AShip::OnIdleTick(float DeltaTime)
{
	StatisticsExporter::Get().IdleTime += DeltaTime;
	
	if(FailedPlanCooldown <= MaxIdleTime)
	{
		if(bUseGOAP)
		{
			//Change the bForwardSearch parameter in this function call to false if using backwards planning
			if(GOAPPlanner::Plan(this, true))
			{
				FString Plan = "";
				for (int i = 0; i < PlannedActions.Num(); i++)
				{
					Plan += PlannedActions[i]->GetName();
					if (i < PlannedActions.Num() - 1)
					{
						Plan += " -> ";
					}
				}
				UE_LOG(LogTemp, Warning, TEXT("%s has found a plan: %s. Executing plan!"), *GetName(), *Plan);
				ChangeState(State_Execute);
			}
			else
			{
				FailedPlanCooldown = MaxIdleTime;
				UE_LOG(LogTemp, Warning, TEXT("%s was unable to find a plan. Idling for %f seconds"), *GetName(), MaxIdleTime);
			}
		}
		else
		{
			if(FSMPlan())
			{
				UE_LOG(LogTemp, Warning, TEXT("%s has found a plan. Executing plan: %s!"), *GetName(), *PlannedActions[0]->GetName());
				ChangeState(State_Execute);
			}
			else
			{
				FailedPlanCooldown = 1;
				bAtGoal = true;
				bAtNextNode = true;
				LevelGenerator->CheckForCollisions();
				UE_LOG(LogTemp, Warning, TEXT("%s was unable to find a plan. Idling for %f seconds"), *GetName(), MaxIdleTime);
			}
		}
		
	}
	else
	{
		FailedPlanCooldown -= DeltaTime;
	}
}

void AShip::OnIdleExit()
{
}

void AShip::OnMoveEnter()
{
	if(PlannedActions.IsEmpty())
	{
		ChangeState(State_Idle);
		return;
	}

	UHLAction* CurrentAction = PlannedActions[0];

	if(CurrentAction->Target == nullptr)
	{
		ChangeState(State_Idle);
		return;
	}

	if(CurrentAction->RequiresInRange())
	{
		GridNode* GoalLocation = LevelGenerator->FindGridNode(CurrentAction->Target);
		if(GoalLocation)
		{
			GoalNode = GoalLocation;
			LevelGenerator->CalculatePath(this, GoalLocation);
			LevelGenerator->CheckForCollisions(); //
			StatisticsExporter::Get().PathLength += Path.Num(); //
			StatisticsExporter::Get().PathCount++; //
		}
		else
		{
			ChangeState(State_Idle);
			return;
		}
	}
	else
	{
		ChangeState(State_Execute);
		return;
	}
}

void AShip::OnMoveTick(float DeltaTime)
{
	UHLAction* CurrentAction = PlannedActions[0];
   
	if(Path.Num() > 0)
	{
		bAtGoal = false;
		bRecentlyCrashed = false;
		if(!bAtNextNode)
		{
			FVector CurrentPosition = GetActorLocation();

			float TargetXPos = Path[0]->X * ALevelGenerator::GRID_SIZE_WORLD;
			float TargetYPos = Path[0]->Y * ALevelGenerator::GRID_SIZE_WORLD;

			FVector TargetPosition(TargetXPos, TargetYPos, CurrentPosition.Z);

			Direction = TargetPosition - CurrentPosition;
			Direction.Normalize();

			CurrentPosition += Direction * MoveSpeed * DeltaTime;

			if(FVector::Dist(CurrentPosition, TargetPosition) <= Tolerance)
			{
				CurrentPosition = TargetPosition;
         
				if(Path[0] == GoalNode)
				{
					bAtGoal = true;
				}

				LastNode = CurrentNode;
				CurrentNode = Path[0];
				Path.RemoveAt(0);
				bAtNextNode = true;
				LevelGenerator->CheckForCollisions();
			}
      
			SetActorLocation(CurrentPosition);
			SetActorRotation(Direction.Rotation());
		}
      
	}
	else
	{
		bAtNextNode = true;
		bAtGoal = true;
		LevelGenerator->CheckForCollisions();
		for(int i = 0; i < PathDisplayActors.Num(); i++)
		{
			if(PathDisplayActors[i])
			{
				PathDisplayActors[i]->Destroy();
			}
		}
		CurrentAction->bInRange = true;
		ChangeState(State_Execute);
	}

   
}

void AShip::OnMoveExit()
{
}

void AShip::OnExecuteEnter()
{
	bAtNextNode = true;

}

void AShip::OnExecuteTick(float DeltaTime)
{
	if(PlannedActions.IsEmpty())
	{
		ChangeState(State_Idle);
		return;
	}

	UHLAction* CurrentAction = PlannedActions[0];

	while(!PlannedActions.IsEmpty())
	{
		if(CurrentAction->IsActionDone())
		{
			PlannedActions.Remove(CurrentAction);
			if(!PlannedActions.IsEmpty())
			{
				CurrentAction = PlannedActions[0];
			}
			for(AActor* PathObject : PathDisplayActors)
			{
				if(PathObject)
				{
					PathObject->Destroy();
				}
			}
			PathDisplayActors.Empty();
		}
		else
		{
			break;
		}
	}
	
	
	if(!PlannedActions.IsEmpty())
	{
		CurrentAction = PlannedActions[0];

		bool InRange = CurrentAction->RequiresInRange() ? CurrentAction->bInRange : true;

		if(InRange)
		{
			bool IsActionSuccessful = CurrentAction->Execute(this, DeltaTime);

			if(!IsActionSuccessful)
			{
				// only change plan if it is really aborted.
				if (OnPlanAborted(CurrentAction))
				{
					ChangeState(State_Idle);
				}
				
			}
		}
		else
		{
			ChangeState(State_Move);
		}
	}
	else
	{
		ChangeState(State_Idle);
	}
}

void AShip::OnExecuteExit()
{
	
}


void AShip::ChangeState(ACTOR_STATES NextState)
{
	switch (CurrentState)
	{
	case State_Idle:
		OnIdleExit();
		break;
	case State_Move:
		OnMoveExit();
		break;
	case State_Execute:
		OnExecuteExit();
		break;
	}

	switch (NextState)
	{
	case State_Idle:
		OnIdleEnter();
		break;
	case State_Move:
		OnMoveEnter();
		break;
	case State_Execute:
		OnExecuteEnter();
		break;
	}

	CurrentState = NextState;
}



TMap<STATE_KEY, int> AShip::GetWorldState()
{
	TMap<STATE_KEY, int> WorldState;

	//Feel free to add more things to your WorldState if you want to use them
	
	WorldState.Add(AgentWood, NumWood);
	WorldState.Add(HasWood, NumWood > 0);
	WorldState.Add(AgentStone, NumStone);
	WorldState.Add(HasStone, NumStone > 0);
	WorldState.Add(AgentGrain, NumGrain);
	WorldState.Add(HasGrain, NumGrain > 0);
	
	TArray<AActor*> Resources;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResource::StaticClass(), Resources);
	TArray<AActor*> Buildings;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), Buildings);

	TArray<AResource*> Lumberyards;
	TArray<AResource*> Mines;
	TArray<AResource*> Farms;
	TArray<AResource*> BuildingSlots;
	TArray<AResource*> Taverns;

	for(int i = 0; i < Resources.Num(); i++)
	{
		AResource* TempResource = Cast<AResource>(Resources[i]);
		if(TempResource)
		{
			switch (TempResource->ResourceType)
			{
				case(GRID_TYPE::Wood):
					Lumberyards.Add(TempResource);
					break;
				case(GRID_TYPE::Stone):
					Mines.Add(TempResource);
					break;
				case(GRID_TYPE::Grain):
					Farms.Add(TempResource);
					break;
				case(GRID_TYPE::BuildingSlot):
					BuildingSlots.Add(TempResource);
					break;
				default:
					break;
			}
		}
	}

	WorldState.Add(NumLumberyards, Lumberyards.Num());
	WorldState.Add(NumMines, Mines.Num());
	WorldState.Add(NumFarms, Farms.Num());
	WorldState.Add(NumBuildingSlots, BuildingSlots.Num());
	WorldState.Add(NumBuildings, Buildings.Num());

	WorldState.Add(NumPoints, LevelGenerator->Points);
	WorldState.Add(TotalWood, LevelGenerator->TotalWood);
	WorldState.Add(TotalStone, LevelGenerator->TotalStone);
	WorldState.Add(TotalGrain, LevelGenerator->TotalGrain);
	WorldState.Add(TimeLeft, LevelGenerator->TimeLimit - LevelGenerator->TimePassed);

	//
	GridNode* Current = LevelGenerator->GetNode(this);
	WorldState.Add(AgentLocationX, Current->X);
	WorldState.Add(AgentLocationY, Current->Y);
	//
	
	return WorldState;

}

//-----------------------------------------YOUR CODE HERE------------------------------------------------------//
TArray<TTuple<STATE_KEY, int, char>> AShip::PickGoal()
{
	TArray<TTuple<STATE_KEY, int, char>> GoalToPersue;

	auto WorldState = GetWorldState();

	if (AgentType == Builder && WorldState[TotalWood] > 15 && WorldState[TotalStone] > 10 && WorldState[TotalGrain] > 5)
	{
		GoalToPersue.Add(TTuple<STATE_KEY, int, char>(NumBuildings, WorldState[NumBuildings], '>'));
		GoalToPersue.Add(TTuple<STATE_KEY, int, char>(NumBuildingSlots, WorldState[NumBuildingSlots], '<'));
		GoalToPersue.Add(TTuple<STATE_KEY, int, char>(NumPoints, WorldState[NumPoints], '>'));
		GoalToPersue.Add(TTuple<STATE_KEY, int, char>(TotalWood, WorldState[TotalWood], '<'));
		GoalToPersue.Add(TTuple<STATE_KEY, int, char>(TotalStone, WorldState[TotalStone], '<'));
		GoalToPersue.Add(TTuple<STATE_KEY, int, char>(TotalGrain, WorldState[TotalGrain], '<'));
	}
	else
	{
		ALevelGenerator* LG = LevelGenerator;
		float ShipTypeWeight = 5.0f;
		GRID_TYPE ShipType = GetResourceType();
		TArray<GRID_TYPE> Types;
		float WoodRequired = (LG->TotalWood /*+ LG->PlannedWood*/ - 15.0f) / 15.0f - (ShipType == GRID_TYPE::Wood) * ShipTypeWeight;
		float StoneRequired = (LG->TotalStone /*+ LG->PlannedStone*/ - 10.0f) / 10.0f - (ShipType == GRID_TYPE::Stone) * ShipTypeWeight;
		float GrainRequired = (LG->TotalGrain /*+ LG->PlannedGrain*/ - 5.0f) / 5.0f - (ShipType == GRID_TYPE::Grain) * ShipTypeWeight;
		/*float WoodRequired = (LG->TotalWood - 15.0f - (ShipType == GRID_TYPE::Wood) * ShipTypeWeight) / 15.0;
		float StoneRequired = (LG->TotalStone - 10.0f - (ShipType == GRID_TYPE::Stone) * ShipTypeWeight) / 10.0f;
		float GrainRequired = (LG->TotalGrain - 5.0f - (ShipType == GRID_TYPE::Grain) * ShipTypeWeight) / 5.0f;*/
		float Min = FMath::Min(TArray{WoodRequired, StoneRequired, GrainRequired});
		if (Min == WoodRequired) Types.Add(GRID_TYPE::Wood);
		if (Min == StoneRequired) Types.Add(GRID_TYPE::Stone);
		if (Min == GrainRequired) Types.Add(GRID_TYPE::Grain);

		TArray<GRID_TYPE> ResultTypes = TArray{ShipType};
		if (!Types.Contains(ShipType)) ResultTypes = Types; 
	
		auto Target = LevelGenerator->CalculateNearestGoal(LevelGenerator->GetNode(this), ResultTypes, 0, nullptr);

		AResource* Resource = Cast<AResource>(Target);
		switch (Resource->ResourceType)
		{
			case GRID_TYPE::Wood:
				GoalToPersue.Add(TTuple<STATE_KEY, int, char>(TotalWood, WorldState[TotalWood], '>'));
				break;
			case GRID_TYPE::Stone:
				GoalToPersue.Add(TTuple<STATE_KEY, int, char>(TotalStone, WorldState[TotalStone], '>'));
				break;
			case GRID_TYPE::Grain:
				GoalToPersue.Add(TTuple<STATE_KEY, int, char>(TotalGrain, WorldState[TotalGrain], '>'));
				break;
			default:
				break;
		}
	}
	
	return GoalToPersue;
}

void AShip::OnPlanFailed(TMap<STATE_KEY, int> FailedGoalState)
{
}

bool AShip::OnPlanAborted(UHLAction* FailedAction)
{
	bool Result = FailedAction->OnActionAborted(this);
	if (!Result)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has aborted the plan. The action %s has failed."), *GetName(), *FailedAction->GetName());
	}
	return Result;
}

void AShip::AddActions()
{
	/*if (AgentType == Builder)
	{
		AvailableActions.Add(UBuildAction::StaticClass());
	}*/
	AvailableActions.Add(UBuildUniversityAction::StaticClass());
	AvailableActions.Add(UBuildTheatreAction::StaticClass());
	AvailableActions.Add(UBuildMarketAction::StaticClass());
	
	AvailableActions.Add(UCollectWoodAction::StaticClass());
	AvailableActions.Add(UCollectStoneAction::StaticClass());
	AvailableActions.Add(UCollectGrainAction::StaticClass());
	
	AvailableActions.Add(UDepositAction::StaticClass());
}

bool AShip::FSMPlan()
{
	bool bPlanFound = false;
	//Make a plan! Then change bPlanFound to true or false depending if a plan was found or not!

	for (int i = 0; i < AvailableActions.Num(); i++)
	{
		UHLAction* NewAction = NewObject<UHLAction>(this, AvailableActions[i]);
		if (NewAction->CheckPreconditions(this, GetWorldState()))
		{
			if (!NewAction->SetupAction(this))
			{
				continue;
			}
			PlannedActions.Add(NewAction);
			bPlanFound = true;
			break;
		}
	}
	
	return bPlanFound;
}

GRID_TYPE AShip::GetResourceType()
{
	GRID_TYPE Result;
	switch (AgentType)
	{
	case Woodcutter:
		Result = GRID_TYPE::Wood;
		break;
	case Stonemason:
		Result = GRID_TYPE::Stone;
		break;
	case Farmer:
		Result = GRID_TYPE::Grain;
		break;
	default: 
		Result = GRID_TYPE::Wood;
		break;
	}

	return Result;
}

float AShip::GetTravelTime(int PathCount)
{
	return PathCount / MoveSpeed * 100;
}
