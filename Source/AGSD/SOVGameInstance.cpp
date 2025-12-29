// Fill out your copyright notice in the Description page of Project Settings.


#include "SOVGameInstance.h"

void USOVGameInstance::UpdatePlotData(FPlotSaveData NewData)
{
	// 장부에 기록 (이미 있으면 덮어쓰기, 없으면 추가)
	GlobalPlotDataMap.Add(NewData.PlotGuid, NewData);
}

bool USOVGameInstance::GetPlotData(FGuid TargetGuid, FPlotSaveData& OutData)
{
	if (FPlotSaveData* FoundData = GlobalPlotDataMap.Find(TargetGuid))
	{
		OutData = *FoundData;
		return true;
	}
	return false;
}
