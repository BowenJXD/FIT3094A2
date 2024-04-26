#include "StatisticExporter.h"

FString StatisticsExporter::GetPath(FString FileName)
{
	return FPaths::ProjectContentDir() + "Logs/" + FileName + ".csv";
}

void StatisticsExporter::Log()
{
	FString Path = GetPath("Log");
    FString Text;

    // Load existing CSV file if it exists
    if (!FFileHelper::LoadFileToString(Text, *Path))
    {
        Text = "Time,Description,Path Length,Path Count,Average Path Length,Wood,Stone,Grain,Actual Uni,Exp Uni,Variance,Resource Point,Building Point,Total Point,Idle Time,Move Time,";
        for (auto& Pair : ExecuteTimes)
        {
            Text += Pair.Key + " Time,";
        }
        Text += "\n";
    }

    int ResourcePoint = Wood * 1 + Stone * 2 + Grain * 3;
    int BuildingPoint = University * 1000;
    float ExpUni = FMath::Min(TArray{Wood / 15, Stone / 10, Grain / 5});
    float Variance = Stone / 10 + Grain / 5 - 2 * Wood / 15;
    Text += FString::Printf(
        TEXT("%s,,%d,%d,%d,%d,%d,%d,%d,%f,%f,%d,%d,%d,%f,%f,"),
        *FDateTime::Now().ToString(),
        PathLength, PathCount, PathLength / PathCount,
        Wood, Stone, Grain, University, ExpUni, Variance,
        ResourcePoint, BuildingPoint,
        ResourcePoint + BuildingPoint,
        IdleTime, MoveTime);
    for (auto& Pair : ExecuteTimes)
    {
        Text += FString::SanitizeFloat(Pair.Value) + ",";
    }
    Text += "\n";

    if (FFileHelper::SaveStringToFile(Text, *Path))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("File Successfully Saved At %s "), *Path));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("File Failed to save At %s "), *Path));
    }

    Clear();
}

void StatisticsExporter::Clear()
{
    PathLength = 0;
    PathCount = 0;
    Wood = 0;
    Stone = 0;
    Grain = 0;
    University = 0;
    IdleTime = 0;
    MoveTime = 0;
    ExecuteTimes.Empty();
}

