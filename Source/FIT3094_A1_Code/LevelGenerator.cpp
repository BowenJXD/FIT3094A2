// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

#include "FIT3094_A1_CodeGameModeBase.h"
#include "Ship.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(Collisions);

// Sets default values
ALevelGenerator::ALevelGenerator()
{
 	// Set this actor to call Tick() every frame. 
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	AFIT3094_A1_CodeGameModeBase* GameModeBase = Cast<AFIT3094_A1_CodeGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GenerateWorldFromFile(GameModeBase->GetMapArray(GameModeBase->GetAssessedMapFile()));
	GenerateScenarioFromFile(GameModeBase->GetMapArray(GameModeBase->GetScenarioFile()));
	InitialiseLevel();
	StartTime = FPlatformTime::Seconds();
}

// Called every frame
void ALevelGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	int TimePassed = FPlatformTime::Seconds() - StartTime;
	int TimeLeft = TimeLimit - TimePassed;

	if(TimeLeft <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("You got %d points!"), Points);
		UE_LOG(LogTemp, Warning, TEXT("Your final Ratio of (Replans-Crashes)/Replans: %f"),((float)(NumReplans - NumCollisions)) / (float)NumReplans);
		UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, true);
	}

	GEngine->AddOnScreenDebugMessage(5, 15, FColor::Red, FString::Printf(TEXT("Time Left: %d"), TimeLeft));
	GEngine->AddOnScreenDebugMessage(1, 15, FColor::Yellow, FString::Printf(TEXT("Amount of Grain: %d"), TotalGrain));
	GEngine->AddOnScreenDebugMessage(2, 15, FColor::White, FString::Printf(TEXT("Amount of Stone: %d"), TotalStone));
	GEngine->AddOnScreenDebugMessage(3, 15, FColor::Orange, FString::Printf(TEXT("Amount of Wood: %d"), TotalWood));
	GEngine->AddOnScreenDebugMessage(4, 15, FColor::Green, FString::Printf(TEXT("Points: %d"), Points));
	if(CollisionAndReplanning)
	{
		if(NumReplans > 0)
		{
			GEngine->AddOnScreenDebugMessage(6, 15, FColor::Cyan, FString::Printf(TEXT("Ratio of (Replans-Crashes)/Replans: %f"), ((float)(NumReplans - NumCollisions)) / (float)NumReplans));
		}
	}
	
}

void ALevelGenerator::SpawnWorldActors(TArray<TArray<char>> Grid)
{
	int WorldStartTime = FPlatformTime::Seconds();
	
    if(DeepBlueprint && ShallowBlueprint && LandBlueprint)
    {
    	for(int Y = 0; Y < MapSizeY; Y++)
    	{
    		for(int X = 0; X < MapSizeX; X++)
    		{
    			bool bCanGoRight = true;
    			int CurrentLocation = X;
    			while(bCanGoRight)
    			{
    				if(CurrentLocation + 1 < MapSizeX)
    				{
    					if(Grid[Y][CurrentLocation] == Grid[Y][CurrentLocation + 1])
    					{
    						CurrentLocation++;
    					}
    					else
    					{
    						bCanGoRight = false;
    					}
    				}
    				else
    				{
    					bCanGoRight = false;
    				}
    			}
    			
    			float XPos = (X + CurrentLocation + 1) / 2 * GRID_SIZE_WORLD;
    			if((X + CurrentLocation + 1) % 2 == 0)
    			{
    				XPos -= GRID_SIZE_WORLD / 2;
    			}
    			float YPos = Y * GRID_SIZE_WORLD;
    
    			FVector Position(XPos, YPos, 0);
    
    			switch(Grid[Y][X])
    			{
    			case '.':
    				{
    					AActor* SpawnedActor = GetWorld()->SpawnActor(DeepBlueprint, &Position);
    					SpawnedActor->SetActorScale3D(FVector((abs(X - CurrentLocation) + 1),1,1));
    					Terrain.Add(SpawnedActor);
    					break;
    				}
    			case 'T':
    				{
    					AActor* SpawnedActor = GetWorld()->SpawnActor(ShallowBlueprint, &Position);
    					SpawnedActor->SetActorScale3D(FVector((abs(X - CurrentLocation) + 1),1,1));
    					Terrain.Add(SpawnedActor);
    					break;
    				}
    			case '@':
    				{
    					AActor* SpawnedActor = GetWorld()->SpawnActor(LandBlueprint, &Position);
    					SpawnedActor->SetActorScale3D(FVector((abs(X - CurrentLocation) + 1),1,1));
    					Terrain.Add(SpawnedActor);
    					break;
    				}
    				
    			default:
    				break;
    			}
    			X += (abs(X - CurrentLocation));
    		}
    	}
    }
    int WorldEndTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("World Gen Took %d Seconds"), (WorldEndTime - WorldStartTime));

	if(Camera)
	{
		FVector CameraPosition = Camera->GetActorLocation();

		CameraPosition.X = MapSizeX * 0.5 * GRID_SIZE_WORLD;
		CameraPosition.Y = MapSizeY * 0.5 * GRID_SIZE_WORLD;

		if(!bCameraRotated)
		{
			bCameraRotated = true;
			FRotator CameraRotation = Camera->GetActorRotation();

			CameraRotation.Pitch = 270;
			CameraRotation.Roll = 180;

			Camera->SetActorRotation(CameraRotation);
			Camera->AddActorLocalRotation(FRotator(0,0,90));
		}
		
		Camera->SetActorLocation(CameraPosition);
		
	}
}

void ALevelGenerator::GenerateNodeGrid(TArray<TArray<char>> Grid)
{
	for(int Y = 0; Y < MapSizeY; Y++)
	{
		for(int X = 0; X < MapSizeX; X++)
		{
			WorldArray[Y][X] = new GridNode();
			WorldArray[Y][X]->Y = Y;
			WorldArray[Y][X]->X = X;

			switch(Grid[Y][X])
			{
			case '.':
				WorldArray[Y][X]->GridType = GRID_TYPE::DeepWater;
				break;
			case '@':
				WorldArray[Y][X]->GridType = GRID_TYPE::Land;
				break;
			case 'T':
				WorldArray[Y][X]->GridType = GRID_TYPE::ShallowWater;
				break;
			default:
				break;
			}
			
		}
	}
}

void ALevelGenerator::ResetAllNodes()
{
	for( int Y = 0; Y < MapSizeY; Y++)
	{
		for(int X = 0; X < MapSizeX; X++)
		{
			WorldArray[Y][X]->F = 0;
			WorldArray[Y][X]->G = 0;
			WorldArray[Y][X]->H = 0;
			WorldArray[Y][X]->Parent = nullptr;
			WorldArray[Y][X]->bIsInClosed = false;
			WorldArray[Y][X]->bIsInOpen = false;
		}
	}
}

int ALevelGenerator::CalculateManhattanDistanceBetween(GridNode* First, GridNode* Second)
{
	return abs(First->X - Second->X) + abs(First->Y - Second->Y);
}

void ALevelGenerator::GenerateWorldFromFile(TArray<FString> WorldArrayStrings)
{
	if(WorldArrayStrings.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Map file not found!"))
		return;
	}

	FString Height = WorldArrayStrings[1];
	Height.RemoveFromStart("height ");
	MapSizeY = FCString::Atoi(*Height);

	FString Width = WorldArrayStrings[2];
	Width.RemoveFromStart("width ");
	MapSizeX = FCString::Atoi(*Width);

	TArray<TArray<char>> CharMapArray;
	CharMapArray.Init( TArray<char>(), MAX_MAP_SIZE);
	
	for(int i = 0; i < CharMapArray.Num(); i++)
	{
		CharMapArray[i].Init('x', MAX_MAP_SIZE);
	}
	
	for(int LineNum = 4; LineNum < MapSizeY + 4; LineNum++)
	{
		for(int CharNum = 0; CharNum < MapSizeX; CharNum++)
		{
			CharMapArray[LineNum-4][CharNum] = WorldArrayStrings[LineNum][CharNum];
		}
	}

	GenerateNodeGrid(CharMapArray);
	SpawnWorldActors(CharMapArray);
	
}

void ALevelGenerator::GenerateScenarioFromFile(TArray<FString> ScenarioArrayStrings)
{
	if(ScenarioArrayStrings.Num() == 0)
	{
		return;
	}
	
	for(int i = 1; i < ScenarioArrayStrings.Num(); i++)
	{
		TArray<FString> SplitLine;
		FString CurrentLine = ScenarioArrayStrings[i];
		
		CurrentLine.ParseIntoArray(SplitLine,TEXT("\t"));

		int ShipX = FCString::Atoi(*SplitLine[4]);
		int ShipY = FCString::Atoi(*SplitLine[5]);
		int ResourceX = FCString::Atoi(*SplitLine[6]);
		int ResourceY = FCString::Atoi(*SplitLine[7]);

		ShipSpawns.Add(FVector2d(ShipX, ShipY));
		ResourceSpawns.Add(FVector2d(ResourceX, ResourceY));
	}
}

void ALevelGenerator::InitialisePaths()
{
	ResetPath();
}

void ALevelGenerator::RenderPath(AShip* Ship)
{
	GridNode* CurrentNode = Ship->GoalNode;

	if(CurrentNode)
	{
		while(CurrentNode->Parent != nullptr)
		{
			FVector Position(CurrentNode->X * GRID_SIZE_WORLD, CurrentNode->Y * GRID_SIZE_WORLD, 10);
			AActor* PathActor = GetWorld()->SpawnActor(PathDisplayBlueprint, &Position);
			PathDisplayActors.Add(PathActor);

			Ship->Path.EmplaceAt(0, WorldArray[CurrentNode->Y][CurrentNode->X]);
			Ship->PathDisplayActors.Add(PathActor);
			CurrentNode = CurrentNode->Parent;
		}
	}
}

void ALevelGenerator::ResetPath()
{
	ResetAllNodes();

	for(int i = 0; i < PathDisplayActors.Num(); i++)
	{
		PathDisplayActors[i]->Destroy();
	}
	PathDisplayActors.Empty();

	for(int i = 0; i < Ships.Num(); i++)
	{
		Ships[i]->Path.Empty();
	}
}

void ALevelGenerator::InitialiseLevel()
{
	DestroyAllActors();

	//Spawn ships
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < ShipsToSpawn[i]; j++)
		{
			int ShipXPos = ShipSpawns[j + TotalIndex].X;
			int ShipYPos = ShipSpawns[j + TotalIndex].Y;
			TotalIndex++;
			FVector ShipPosition(ShipXPos* GRID_SIZE_WORLD, ShipYPos* GRID_SIZE_WORLD, 20);
			AShip* Ship = Cast<AShip>(GetWorld()->SpawnActor(ShipBlueprint, &ShipPosition));
			
			Ships.Add(Ship);
		
			switch(i)
			{
			case 0:
				Ship->AgentType = AShip::Woodcutter;
				break;
			case 1:
				Ship->AgentType = AShip::Stonemason;
				break;
			case 2:
				Ship->AgentType = AShip::Farmer;
				break;
			case 3:
				Ship->AgentType = AShip::Builder;
				break;
			}
			
			switch(Ship->AgentType)
			{
			case AShip::Woodcutter:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->WoodCutterMaterial, Ship);
				Ship->WoodCutterMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);
				break;
			case AShip::Stonemason:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->StoneMasonMaterial, Ship);
				Ship->StoneMasonMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);

				break;
			case AShip::Farmer:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->FarmerMaterial, Ship);
				Ship->FarmerMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);

				break;
			case AShip::Builder:
				Ship->MaterialInstance = UMaterialInstanceDynamic::Create(Ship->BuilderMaterial, Ship);
				Ship->BuilderMaterial->GetVectorParameterValue(FName(TEXT("Colour")), Ship->OldColour);
				break;
			}
			
			for(int k = 0; k < Ship->Meshes.Num(); k++)
			{
				Cast<UStaticMeshComponent>(Ship->Meshes[k])->SetMaterial(0, Ship->MaterialInstance);
			}
		}
		
	}

	for(int i  = 0; i < 5; i++)
	{
		for(int j = 0; j < ResourcesToSpawn[i]; j++)
		{
			int ResourceXPos = ResourceSpawns[j + TotalIndex].X;
			int ResourceYPos = ResourceSpawns[j + TotalIndex].Y;
			TotalIndex++;

			FVector ResourcePosition(ResourceXPos* GRID_SIZE_WORLD, ResourceYPos* GRID_SIZE_WORLD, 20);
			AResource* Resource = Cast<AResource>(GetWorld()->SpawnActor(GrainBlueprint, &ResourcePosition));
			Resource->XPos = ResourceXPos;
			Resource->YPos = ResourceYPos;
			if(ResourceXPos >= MapSizeX || ResourceYPos >= MapSizeY)
			{
				continue;
			}
			WorldArray[ResourceYPos][ResourceXPos]->ResourceAtLocation = Resource;

			switch(i)
			{
			case 0:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Home;
				Resource->ResourceType = GRID_TYPE::Home;
				break;
			case 1:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Wood;
				Resource->ResourceType = GRID_TYPE::Wood;
				break;
			case 2:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Stone;
				Resource->ResourceType = GRID_TYPE::Stone;
				break;
			case 3:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::Grain;
				Resource->ResourceType = GRID_TYPE::Grain;
				break;
			case 4:
				WorldArray[ResourceYPos][ResourceXPos]->GridType = GRID_TYPE::BuildingSlot;
				Resource->ResourceType = GRID_TYPE::BuildingSlot;
				break;
			}
			Resource->ChangeMaterial();
		}
	}
	
	InitialisePaths();
}

void ALevelGenerator::DestroyAllActors()
{
	for(int i = 0; i < PathDisplayActors.Num(); i++)
	{
		PathDisplayActors[i]->Destroy();
	}
	PathDisplayActors.Empty();
	
	for(int i = 0; i < Ships.Num(); i++)
	{
		Ships[i]->Destroy();
	}
	Ships.Empty();
	
}

void ALevelGenerator::CheckForCollisions()
{
	for(int i = 0; i < Ships.Num(); i++)
	{
		if(!Ships[i]->bAtNextNode)
		{
			return;
		}
	}

	if(!CollisionAndReplanning)
	{
		for(int i = 0; i < Ships.Num(); i++)
		{
			Ships[i]->bAtNextNode = Ships[i]->Path.Num() == 0;
		}
		return;
	}
	TArray<GridNode*> LastNodes;
	TArray<GridNode*> CurrentNodes;
	TArray<GridNode*> NextNodes;
	for(int i = 0; i < Ships.Num(); i++)
	{
		Ships[i]->bAtNextNode = Ships[i]->Path.Num() == 0;
		LastNodes.Add(Ships[i]->LastNode);
		CurrentNodes.Add(Ships[i]->CurrentNode);
		if(Ships[i]->bAtGoal == true)
		{
			NextNodes.Add(Ships[i]->GoalNode);
		}
		else
		{
			NextNodes.Add(Ships[i]->Path[0]);
		}
	}

	for(int i = 0; i < NextNodes.Num(); i++)
	{
		for(int j = i + 1; j < NextNodes.Num(); j++)
		{
			if(NextNodes[i] == NextNodes[j])
			{
				if(!(Ships[i]->bAtGoal && Ships[j]->bAtGoal))
				{
					UE_LOG(Collisions, Warning, TEXT("CRASH GOING TO OCCUR AT %d %d"), NextNodes[i]->X, NextNodes[i]->Y);
					NumReplans++;
					//REPLAN HERE
				}
				else
				{
					UE_LOG(Collisions, Warning, TEXT("Crash Avoided because both ships not moving"));
				}
			}
		}
		for(int j = i + 1; j < CurrentNodes.Num(); j++)
		{
			if(NextNodes[i] == CurrentNodes[j])
			{
				if(NextNodes[j] == CurrentNodes[i])
				{
					if(!(Ships[i]->bAtGoal && Ships[j]->bAtGoal))
					{
						UE_LOG(Collisions, Warning, TEXT("CRASH GOING TO OCCUR AT %d %d"), NextNodes[i]->X, NextNodes[i]->Y);
						NumReplans++;
						//REPLAN HERE
					}
				}
			}
		}
	}

	for(int i = 0; i < CurrentNodes.Num(); i++)
	{
		for(int j = i + 1; j < CurrentNodes.Num(); j++)
		{
			if(CurrentNodes[i] == CurrentNodes[j])
			{
				if(!(Ships[i]->bAtGoal && Ships[j]->bAtGoal))
				{
					if(!Ships[i]->bRecentlyCrashed || !Ships[j]->bRecentlyCrashed)
					{
						Ships[i]->bRecentlyCrashed = true;
						Ships[j]->bRecentlyCrashed = true;
						UE_LOG(Collisions, Warning, TEXT("CRASH OCCURED AT %d %d"), CurrentNodes[i]->X, CurrentNodes[i]->Y);
						NumCollisions++;
					}
				}
			}
		}
		for(int j = i + 1; j < LastNodes.Num(); j++)
		{
			if(CurrentNodes[i] == LastNodes[j])
			{
				if(CurrentNodes[j] == LastNodes[i])
				{
					if(!(Ships[i]->bAtGoal && Ships[j]->bAtGoal))
					{
						if(!Ships[i]->bRecentlyCrashed || !Ships[j]->bRecentlyCrashed)
						{
							Ships[i]->bRecentlyCrashed = true;
							Ships[j]->bRecentlyCrashed = true;
							UE_LOG(Collisions, Warning, TEXT("CRASH OCCURED AT %d %d"), CurrentNodes[i]->X, CurrentNodes[i]->Y);
							NumCollisions++;
						}
					}
				}
			}
		}
	}
}

void ALevelGenerator::CalculatePath(AShip* Ship, GridNode* Resource)
{
	priority_queue<GridNode*, vector<GridNode*>, CompareGridNodes> OpenQueue;
	
	ResetAllNodes();
		
	GridNode* CurrentNode = WorldArray[(int)Ship->GetActorLocation().Y / GRID_SIZE_WORLD][(int)Ship->GetActorLocation().X / GRID_SIZE_WORLD];
	CurrentNode->G = 0;
	CurrentNode->H = CalculateManhattanDistanceBetween(CurrentNode, Ship->GoalNode);
	CurrentNode->F = CurrentNode->G + CurrentNode->H;
	CurrentNode->bIsInOpen = true;
	OpenQueue.push(CurrentNode);
		
	while(OpenQueue.size() > 0)
	{
		CurrentNode = OpenQueue.top();
		OpenQueue.pop();
		CurrentNode->bIsInClosed = true;
			
		if(CurrentNode == Resource){
			//GOAL FOUND
			RenderPath(Ship);
			break;
		}
		
		TArray<GridNode*> Neighbours = GetNeighbours(CurrentNode);
			
		for(int j = 0; j < Neighbours.Num(); j++){
			GridNode* CurrentNeighbour = Neighbours[j];
			if(CurrentNeighbour->bIsInClosed){
				continue;
			}
			bool bPossibleGIsBetter = false;
			int PossibleG = CurrentNode->G + CurrentNeighbour->GetTravelCost();
			if(!CurrentNeighbour->bIsInOpen)
			{
				bPossibleGIsBetter = true;
			}
			else
			{
				if(PossibleG < CurrentNeighbour->G)
				{
					bPossibleGIsBetter = true;
				}
			}
			
			if(bPossibleGIsBetter)
			{
				CurrentNeighbour->Parent = CurrentNode;
				CurrentNeighbour->G = PossibleG;
				CurrentNeighbour->H = CalculateManhattanDistanceBetween(CurrentNeighbour, Ship->GoalNode);
				CurrentNeighbour->F = CurrentNeighbour->G + CurrentNeighbour->H;
				CurrentNeighbour->bIsInOpen = true;
				OpenQueue.push(CurrentNeighbour);
			}
			
		}
	}
}

TArray<GridNode*> ALevelGenerator::GetNeighbours(GridNode* NodeToCheck){
	TArray<GridNode*> Neighbours;
	
	if(NodeToCheck->X > 0){
		Neighbours.Add(WorldArray[NodeToCheck->Y][NodeToCheck->X - 1]);
	}
	
	if(NodeToCheck->X < MapSizeX - 1){
		Neighbours.Add(WorldArray[NodeToCheck->Y][NodeToCheck->X + 1]);		
	}
	
	if(NodeToCheck->Y > 0){
		Neighbours.Add(WorldArray[NodeToCheck->Y - 1][NodeToCheck->X]);
	}
	
	if(NodeToCheck->Y < MapSizeY - 1){
		Neighbours.Add(WorldArray[NodeToCheck->Y + 1][NodeToCheck->X]);		
	}
	
	return Neighbours;
	
}

AActor* ALevelGenerator::CalculateNearestGoal(AActor* Ship, GRID_TYPE ResourceType)
{
	float ShortestPath = 999999;

	AActor* ClosestResource = nullptr;

	TArray<AActor*> Resources;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResource::StaticClass(), Resources);
	for(AActor* Resource : Resources)
	{
		if(!IsValid(Resource) || Resource->GetName().Contains("Path"))
		{
			continue;
		}
		if(Cast<AResource>(Resource)->ResourceType == ResourceType)
		{
			float CurrentPath = FVector::Dist(Ship->GetActorLocation(), Resource->GetActorLocation());
			if(CurrentPath < ShortestPath)
			{
				ShortestPath = CurrentPath;
				ClosestResource = Resource;
			}
		}
	}
	
	return ClosestResource;
}

GridNode* ALevelGenerator::FindGridNode(AActor* ActorResource)
{
	AResource* Resource = Cast<AResource>(ActorResource);
	if(!Resource)
	{
		return nullptr;
	}
	return WorldArray[Resource->YPos][Resource->XPos];
}

//-----------------------------------YOUR CODE--------------------------------
void ALevelGenerator::Replan(AShip* Ship)
{
	//INSERT REPLANNING HERE
}

bool ALevelGenerator::CollectResource(AShip* Ship, AResource* Resource)
{
	bool Result = false;
	if (!Ship || !Resource) return Result;

	if (Resource->ResourceCount <= 0)
	{
		Resource->Destroy();
		return Result;
	}

	Resource->ResourceCount--;
	switch (Resource->ResourceType)
	{
	case GRID_TYPE::Wood:
		Ship->NumWood++;
		TotalWood++;
		break;
	case GRID_TYPE::Stone:
		Ship->NumStone++;
		TotalStone++;
		break;
	case GRID_TYPE::Grain:
		Ship->NumGrain++;
		TotalGrain++;
		break;
	default:
		break;
	}
	Result = true;
	
	return Result;
}
