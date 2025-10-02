// Fill out your copyright notice in the Description page of Project Settings.


#include "CropManager.h"

// Sets default values
ACropManager::ACropManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void ACropManager::ResisterCrop(ACrop* Crop, int32 NextAdvanceDay)
{
	if (Crop == nullptr || NextAdvanceDay <= 0) return;
	
	FCropArrayWrapper& DaySchedule = ResisterCropArray.FindOrAdd(NextAdvanceDay);

	if (DaySchedule.CropsToAdvance.Contains(Crop))	return;
	
	DaySchedule.CropsToAdvance.Add(Crop);
}

void ACropManager::HandleDayPassed(int32 CurrentDay)
{
	if (CurrentDay <= 0) return;

	if (FCropArrayWrapper* DaySchedule = ResisterCropArray.Find(CurrentDay))
	{
		for (ACrop* Crop : DaySchedule->CropsToAdvance)
		{
			if (Crop == nullptr) continue;
			Crop->AdvanceGrowth();
			if (!Crop->GetFullyGrown())
				ResisterCrop(Crop, Crop->GetScheduledDay());
		}
	}
	ResisterCropArray.Remove(CurrentDay);
}

void ACropManager::UnregisterCrop(ACrop* Crop, int32 ScheduledDay)
{
	if (Crop == nullptr || ScheduledDay <= 0) return;
	
	if(FCropArrayWrapper* DaySchedule = ResisterCropArray.Find(ScheduledDay))
	{
		DaySchedule->CropsToAdvance.RemoveSingleSwap(Crop);

		if (DaySchedule->CropsToAdvance.Num() == 0) ResisterCropArray.Remove(ScheduledDay);
	}
}

// Called when the game starts or when spawned
void ACropManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACropManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

