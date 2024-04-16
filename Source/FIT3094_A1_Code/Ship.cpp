// Fill out your copyright notice in the Description page of Project Settings.
#include "Ship.h"

#include "FindResourceAction.h"
#include "GOAPPlanner.h"
#include "LevelGenerator.h"
#include "HLActions/CollectAction.h"
#include "Kismet/GameplayStatics.h"

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
	if(FailedPlanCooldown <= MaxIdleTime)
	{
		if(bUseGOAP)
		{
			//Change the bForwardSearch parameter in this function call to false if using backwards planning
			if(GOAPPlanner::Plan(this, false))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s has found a plan. Executing plan!"), *GetName());
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
				UE_LOG(LogTemp, Warning, TEXT("%s has found a plan. Executing plan!"), *GetName());
				ChangeState(State_Execute);
			}
			else
			{
				FailedPlanCooldown = 1;
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
				ChangeState(State_Idle);
				OnPlanAborted(CurrentAction);
				
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
	WorldState.Add(TimeLeft, LevelGenerator->TimeLimit);
	
	return WorldState;

}

//-----------------------------------------YOUR CODE HERE------------------------------------------------------//
TArray<TTuple<STATE_KEY, int, char>> AShip::PickGoal()
{
	TArray<TTuple<STATE_KEY, int, char>> GoalToPersue;

	return GoalToPersue;
}

void AShip::OnPlanFailed(TMap<STATE_KEY, int> FailedGoalState)
{
}

void AShip::OnPlanAborted(UHLAction* FailedAction)
{
}

void AShip::AddActions()
{
	AvailableActions.Add(CollectAction::StaticClass());
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
