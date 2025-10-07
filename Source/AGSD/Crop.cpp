// Fill out your copyright notice in the Description page of Project Settings.


#include "Crop.h"

#include "AGSDGameStateBase.h"
#include "CropManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AssetRegistryInterface.h"

// Sets default values
ACrop::ACrop()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//루트 컴포넌트 설정
	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;

	CropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACrop::BeginPlay()
{
	Super::BeginPlay();
}

void ACrop::SetCropData(UUCropData* CData)
{
	if (CData != nullptr)
	{
		this->CropData = CData;
		CurrentGrowStageIndex = 0;
		FinishGrowStageIndex = CData->GrowthStages.Num() - 1;	

		AAGSDGameStateBase* GS = Cast<AAGSDGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
		if (!GS) return;

		int32 CurrentDay = GS->GetCurrentDay();
		
		GrowthTimeCounter = CData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;

		ScheduledDay = CurrentDay + GrowthTimeCounter;
		
		if (ACropManager* Manager = Cast<ACropManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACropManager::StaticClass())))
		{
			Manager->ResisterCrop(this, ScheduledDay);
		}

		MeshUpdate();
	}
}

void ACrop::AdvanceGrowth()
{
	if (CurrentGrowStageIndex < FinishGrowStageIndex)
	{
		CurrentGrowStageIndex++;
		GrowthTimeCounter = CropData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;
		MeshUpdate();
		
		ScheduledDay += GrowthTimeCounter;
	}
	if (CurrentGrowStageIndex >= FinishGrowStageIndex) FullyGrown = true;
}

void ACrop::MeshUpdate()
{
	if (CropData != nullptr && CropData->GrowthStages[CurrentGrowStageIndex].Mesh)
	{
		CropMesh->SetStaticMesh(CropData->GrowthStages[CurrentGrowStageIndex].Mesh);
	}
}

