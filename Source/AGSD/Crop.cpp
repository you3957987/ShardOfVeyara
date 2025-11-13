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

	CropMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECR_Overlap
		);
	CropMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECR_Ignore
		);

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
		IInteraction::Execute_ShowWidget(this, player);
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
	if (HarvestClass == nullptr) return;
	
	const float SpawnRadius = 50.f;
	const FVector TargetLocation = GetActorLocation();

	float RandomAngle = FMath::RandRange(0.0f, 360.f);
	float RandomDist = FMath::RandRange(0.f, SpawnRadius);

	FVector SpawnOffset(
		RandomDist * FMath::Cos(RandomAngle),
		RandomDist * FMath::Sin(RandomAngle),
		0.0f
		);

	FVector FinalSpawnLocation = TargetLocation + SpawnOffset;

	FTransform SpawnTransform = GetTransform();
	SpawnTransform.SetLocation(FinalSpawnLocation + FVector(0.f, 0.f, 40.f));
	
	Aharvest* Harvest = GetWorld()->SpawnActorDeferred<Aharvest>(
	CropData->HarvestRewards[0].Harvest,
	SpawnTransform,
	this,
	nullptr,
	ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (Harvest)
	{
		UGameplayStatics::FinishSpawningActor(Harvest, SpawnTransform);
		//초기 선형 속도 설정: Z축(위) 방향으로만 작은 속도를 줌
		if (Harvest->GetMeshComponent()) // Harvest 액터에 MeshComponent를 가져오는 함수가 있다고 가정
		{
			// 50.0f 정도의 작은 힘으로 위로 튀어 오르게 합니다.
			Harvest->GetMeshComponent()->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 100.0f)); 
		}
	}
}

void ACrop::RegisterCropToManager(int32 growthTimeCounter)
{
	AAGSDGameStateBase* GS = Cast<AAGSDGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));
	if (!GS) return;

	int32 CurrentDay = GS->GetCurrentDay();

	ScheduledDay = CurrentDay + growthTimeCounter;
		
	if (ACropManager* Manager = Cast<ACropManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ACropManager::StaticClass())))
	{
		Manager->ResisterCrop(this, ScheduledDay);
	}

	MeshUpdate(CurrentGrowStageIndex);
}

void ACrop::SetCropData(UUCropData* CData)
{
	if (CData != nullptr)
	{
		this->CropData = CData;
		CurrentGrowStageIndex = 0;
		FinishGrowStageIndex = CData->GrowthStages.Num() - 1;	

		GrowthTimeCounter = CData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;

		RegisterCropToManager(GrowthTimeCounter);
	}
}

//상호작용 시 구현부
void ACrop::Interact_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ACrop::OnBeginOverlap"));

	for (int i = 0; i < CropData->HarvestRewards[0].Quantity; i++)	HarvestCrop();
	Destroy();
}

void ACrop::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

void ACrop::AdvanceGrowth()
{
	if (CurrentGrowStageIndex < FinishGrowStageIndex)
	{
		CurrentGrowStageIndex++;
		GrowthTimeCounter = CropData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;
		MeshUpdate(CurrentGrowStageIndex);
		
		ScheduledDay += GrowthTimeCounter;
	}
	if (CurrentGrowStageIndex >= FinishGrowStageIndex)
	{
		FullyGrown = true;
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ACrop::MeshUpdate(int32 currentGrowStageIndex)
{
	if (CropData != nullptr && CropData->GrowthStages[currentGrowStageIndex].Mesh)
	{
		CropMesh->SetStaticMesh(CropData->GrowthStages[currentGrowStageIndex].Mesh);
	}
}

