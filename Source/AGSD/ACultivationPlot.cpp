// Fill out your copyright notice in the Description page of Project Settings.


#include "ACultivationPlot.h"
#include "Components/BoxComponent.h"
#include "AGSDCharacter.h"
#include "AGSDPlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AACultivationPlot::AACultivationPlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//루트 컴포넌트 설정
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;
	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.f, 50.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AACultivationPlot::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AACultivationPlot::OnEndOverlap);
}

//오버랩 시작 함수 구현부
void AACultivationPlot::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void AACultivationPlot::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

//오버랩 종료 함수 구현부
void AACultivationPlot::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
		if (player->GetInteractableActorNum() > 0) return;
		if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))	PlayerController->HideInteractionWidget();
	}
}

void AACultivationPlot::Interact_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("AACultivationPlot::OnBeginOverlap"));
	
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (PlantedCrop == nullptr)
	{
		PlantCrop();
	}
}



// Called when the game starts or when spawned
void AACultivationPlot::BeginPlay()
{
	Super::BeginPlay();
	
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
	SpawnTransform.SetLocation(GetActorLocation() + FVector(0.f, 0.f, 15.f));

	/*
	float RandomYaw = FMath::RandRange(0.f, 360.f);
	FQuat RandomRotation = FQuat(FRotator(0.f, RandomYaw, 0.0f));
	SpawnTransform.SetRotation(RandomRotation);
	*/
	
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
		PlantedCrop->OnDestroyed.AddDynamic(this, &AACultivationPlot::OnPlantedCropDestroyed);
		UGameplayStatics::FinishSpawningActor(PlantedCrop, SpawnTransform);
	}
}

void AACultivationPlot::OnPlantedCropDestroyed(AActor* DestroyedActor)
{
	PlantedCrop = nullptr;
	if (CollisionBox)
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called every frame
void AACultivationPlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

