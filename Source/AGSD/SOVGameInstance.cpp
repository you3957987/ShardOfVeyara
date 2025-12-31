// Fill out your copyright notice in the Description page of Project Settings.


#include "SOVGameInstance.h"

void USOVGameInstance::UpdatePlotData(FPlotSaveData NewData)
{
	// 장부에 기록 (이미 있으면 덮어쓰기, 없으면 추가)
	GlobalPlotDataMap.Add(NewData.PlotName, NewData);
}

bool USOVGameInstance::GetPlotData(FString name, FPlotSaveData& OutData)
{
	if (FPlotSaveData* FoundData = GlobalPlotDataMap.Find(name))
	{
		OutData = *FoundData;
		return true;
	}
	return false;
}
