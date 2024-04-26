#pragma once
#include "Misc/LazySingleton.h"
/**
* Export Log data to CSV
 */
class StatisticsExporter
{
public:

	static StatisticsExporter& Get()
	{
		return TLazySingleton<StatisticsExporter>::Get();
	}

	friend class FLazySingleton;
	
	static FString GetPath(FString FileName);

	int PathLength;
	int PathCount;

	int Wood;
	int Stone;
	int Grain;
	int University;

	float IdleTime;
	float MoveTime;
	TMap<FString, float> ExecuteTimes;

	void Log();

	void Clear();
};