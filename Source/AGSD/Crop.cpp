// Fill out your copyright notice in the Description page of Project Settings.


#include "Crop.h"

#include "AGSDGameStateBase.h"
#include "CropManager.h"
#include "Components/sphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AGSDCharacter.h"
#include "AGSDPlayerController.h"

// Sets default values
ACrop::ACrop()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//루트 컴포넌트 설정
	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;

	CropMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetSphereRadius(50);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ACrop::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ACrop::OnEndOverlap);
}

//오버랩 시작 함수 구현부
void ACrop::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))	PlayerController->ShowInteractionWidget();
		player->AddInteractableActor(this);
		/*
		if (Implements<UInteraction>())
		{
			IInteraction::Execute_Interact(this);
		}
		*/
	}
}

//오버랩 종료 함수 구현부
void ACrop::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
		if (player->GetInteractableActorNum() > 0) return;
		if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))	PlayerController->HideInteractionWidget();
	}
}

// Called when the game starts or when spawned
void ACrop::BeginPlay()
{
	Super::BeginPlay();
}

//작물 수확 구현부
void ACrop::HarvestCrop()
{
	Destroy();
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

//상호작용 시 구현부
void ACrop::Interact_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ACrop::OnBeginOverlap"));

	HarvestCrop();
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
	if (CurrentGrowStageIndex >= FinishGrowStageIndex)
	{
		FullyGrown = true;
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ACrop::MeshUpdate()
{
	if (CropData != nullptr && CropData->GrowthStages[CurrentGrowStageIndex].Mesh)
	{
		CropMesh->SetStaticMesh(CropData->GrowthStages[CurrentGrowStageIndex].Mesh);
	}
}

