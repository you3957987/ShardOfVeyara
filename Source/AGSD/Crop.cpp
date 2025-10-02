// Fill out your copyright notice in the Description page of Project Settings.


#include "Crop.h"
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
}

// Called when the game starts or when spawned
void ACrop::BeginPlay()
{
	Super::BeginPlay();
	ACropManager* Manager = Cast<ACropManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACropManager::StaticClass()));
	if (Manager != nullptr)
	{
		Manager->ResisterCrop(this, ScheduledDay);
	}
}

void ACrop::SetCropData(UUCropData* CData)
{
	if (CData != nullptr)
	{
		this->CropData = CData;
		CurrentGrowStageIndex = 0;
		GrowthTimeCounter = CData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;

		MeshUpdate();
	}
}

void ACrop::AdvanceGrowth()
{
	if (CurrentGrowStageIndex < 4)
	{
		CurrentGrowStageIndex++;
		GrowthTimeCounter = CropData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;
		MeshUpdate();
		ScheduledDay += GrowthTimeCounter;
	}
	if (CurrentGrowStageIndex >= 4) FullyGrown = true;
}

void ACrop::MeshUpdate()
{
	if (CropData != nullptr && CropData->GrowthStages[CurrentGrowStageIndex].Mesh)
	{
		CropMesh->SetStaticMesh(CropData->GrowthStages[CurrentGrowStageIndex].Mesh);
	}
}

