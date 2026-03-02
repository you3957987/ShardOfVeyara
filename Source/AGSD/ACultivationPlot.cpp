// Fill out your copyright notice in the Description page of Project Settings.


#include "ACultivationPlot.h"
#include "Components/BoxComponent.h"
#include "AGSDPlayerController.h"
#include "AGSDCharacter.h"
#include "SeedDataTable.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AACultivationPlot::AACultivationPlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//루트 컴포넌트 설정
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.f, 50.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AACultivationPlot::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AACultivationPlot::OnEndOverlap);
}

void AACultivationPlot::HandleDayPassed(int32 CurrentDay)
{
	if (PlantedCrop)
	{
		if (bHasWeeds) ScheduledDay++;
		else if (ScheduledDay <= CurrentDay) AdvanceGrowth();
	}
	if (!bHasWeeds && FMath::FRand() <= weedsProb)
	{
		bHasWeeds = true;
		SpawnWeeds();
	}
}

//오버랩 시작 함수 구현부
void AACultivationPlot::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{		
		player->AddInteractableActor(this);
	}
}

void AACultivationPlot::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool AACultivationPlot::CanInteract_Implementation(AAGSDCharacter* player)
{
	if (PlantedCrop == nullptr)	return player->HoldingState == EHoldingState::EHS_Seed;
	return false;
}

//오버랩 종료 함수 구현부
void AACultivationPlot::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

void AACultivationPlot::Interact_Implementation(AAGSDCharacter* player)
{
	if (!SeedDataTable) return;
	UE_LOG(LogTemp, Warning, TEXT("AACultivationPlot::OnBeginOverlap"));
	
	//CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (PlantedCrop == nullptr)
	{
		SeedName = FName(*player->SubItemAmount());
		
		GetSeedInfo(SeedName);
		
		CurrentGrowStageIndex = 0;
		GrowthTimeCounter = CropData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;
		FinishGrowStageIndex = CropData->GrowthStages.Num() - 1;
		
		int32 currentDay = GS->GetCurrentDay();
		ScheduledDay =  currentDay + GrowthTimeCounter;
		
		PlantCrop();
		PlantedCrop->MeshUpdate(CurrentGrowStageIndex);
	}
}

// Called when the game starts or when spawned
void AACultivationPlot::BeginPlay()
{
	Super::BeginPlay();
	GI = Cast<USOVGameInstance>(GetGameInstance());
	GS = Cast<AAGSDGameStateBase>(UGameplayStatics::GetGameState(GetWorld()));

	if (AAGSDGameStateBase* GameState = Cast<AAGSDGameStateBase>(UGameplayStatics::GetGameState(GetWorld())))
	{
		GameState->OnDayChangedDelegate.AddUObject(this, &AACultivationPlot::HandleDayPassed);
	}
	if (!GI) return;
	
	FPlotSaveData LoadedData;
	// 아까 만든 구조체 데이터를 통째로 넘겨서 저장합니다.
	if (!GI->GetPlotData(GetName(), LoadedData)) return;
	if (LoadedData.SeedName == NAME_None) return;

	SeedName = LoadedData.SeedName;
	CurrentGrowStageIndex = LoadedData.CurrentGrowStageIndex;
	GrowthTimeCounter = LoadedData.GrowthTimeCounter;
	FinishGrowStageIndex = LoadedData.FinishGrowStageIndex;
	FullyGrown = LoadedData.FullyGrown;
	ScheduledDay = LoadedData.ScheduledDay;
	bHasWeeds = LoadedData.bHasWeeds;

	
	//CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetSeedInfo(SeedName);
	PlantCrop();
	if (bHasWeeds) SpawnWeeds();

	int32 CurrentDay = GI->CurrentDay;
	while (ScheduledDay <= CurrentDay && !FullyGrown && !bHasWeeds)
	{
		GrowthLogic();
	}
	
	PlantedCrop->MeshUpdate(CurrentGrowStageIndex);

	if (CurrentGrowStageIndex >= FinishGrowStageIndex)
	{
		FullyGrown = true;
		PlantedCrop->SetCollisionEnable();
	}
}

void AACultivationPlot::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	FPlotSaveData SaveData;
	SaveData.PlotName = GetName();
	SaveData.SeedName = SeedName;
	SaveData.CurrentGrowStageIndex = CurrentGrowStageIndex;
	SaveData.GrowthTimeCounter = GrowthTimeCounter;
	SaveData.FinishGrowStageIndex = FinishGrowStageIndex;
	SaveData.FullyGrown = FullyGrown;
	SaveData.ScheduledDay = ScheduledDay;
	SaveData.bHasWeeds = bHasWeeds;

	if (GI)
	{
		// 아까 만든 구조체 데이터를 통째로 넘겨서 저장합니다.
		GI->UpdatePlotData(SaveData);
	}
}

void AACultivationPlot::PlantCrop()
{
	// ★ 1. 에디터에서 작물 블루프린트(CropClassToPlant)를 설정했는지 확인
	if (!CropClassToPlant)
	{
		UE_LOG(LogTemp, Error, TEXT("'%s'에 CropClassToPlant가 설정되지 않았습니다!"), *GetName());
		return;
	}
	
	FTransform SpawnTransform = GetTransform();

	PlantedCrop = GetWorld()->SpawnActorDeferred<ACrop>(
	CropClassToPlant,
	SpawnTransform,
	this,
	nullptr,
	ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (PlantedCrop)
	{
	    PlantedCrop->SetCropData(CropData);

		PlantedCrop->OnHarvested.AddDynamic(this, &AACultivationPlot::OnPlantedCropDestroyed);		
		UGameplayStatics::FinishSpawningActor(PlantedCrop, SpawnTransform);
	}
}

void AACultivationPlot::OnPlantedCropDestroyed()
{
	PlantedCrop = nullptr;
	
	CurrentGrowStageIndex = 0;
	FullyGrown = false;
	ScheduledDay = 0;
	SeedName = NAME_None;

	/*
	if (CollisionBox)
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	*/
}

void AACultivationPlot::GetSeedInfo(FName TargetRowName)
{
	if (!SeedDataTable) return;
	
	static const FString Context(TEXT("Seed Data Context"));

	FSeedData* SeedRow = SeedDataTable->FindRow<FSeedData>(TargetRowName, Context);
	if (SeedRow)
	{
		CropData = SeedRow->CropData.Get();

		if (!CropData) CropData = SeedRow->CropData.LoadSynchronous();
	}
}

void AACultivationPlot::OnWeedRemoved()
{
	bHasWeeds = false;
}

void AACultivationPlot::GrowthLogic() {
	if (CurrentGrowStageIndex < FinishGrowStageIndex)
	{
		CurrentGrowStageIndex++;
		GrowthTimeCounter = CropData->GrowthStages[CurrentGrowStageIndex].TimeToGrow;
		
		ScheduledDay += GrowthTimeCounter;
	}
	if (CurrentGrowStageIndex >= FinishGrowStageIndex)
	{
		FullyGrown = true;
		PlantedCrop->SetCollisionEnable();
	}
}

void AACultivationPlot::SpawnWeeds()
{
	if (!WeedsActorClass) return;

	FTransform SpawnTransform = GetTransform();
	
	WeedsActor = GetWorld()->SpawnActorDeferred<AWeeds>(
	WeedsActorClass,
	SpawnTransform,
	this,
	nullptr,
	ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	UGameplayStatics::FinishSpawningActor(WeedsActor, SpawnTransform);
	WeedsActor->OnWeeding.AddDynamic(this, &AACultivationPlot::OnWeedRemoved);
}

void AACultivationPlot::AdvanceGrowth()
{
	GrowthLogic();
	PlantedCrop->MeshUpdate(CurrentGrowStageIndex);
}

