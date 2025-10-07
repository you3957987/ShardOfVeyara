// Fill out your copyright notice in the Description page of Project Settings.


#include "ACultivationPlot.h"
#include "Components/BoxComponent.h"
#include "AGSDCharacter.h"
#include "AGSDPlayerController.h"
#include "Crop.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AACultivationPlot::AACultivationPlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//루트 컴포넌트 설정
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;
	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(40.0f, 40.f, 40.f));
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AACultivationPlot::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AACultivationPlot::OnEndOverlap);
}

//오버랩 시작 함수 구현부
void AACultivationPlot::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
void AACultivationPlot::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

	FTransform SpawnTransform = GetTransform();
	SpawnTransform.SetLocation(GetActorLocation() + FVector(0.f, 0.f, 15.f));

	/*
	float RandomYaw = FMath::RandRange(0.f, 360.f);
	FQuat RandomRotation = FQuat(FRotator(0.f, RandomYaw, 0.0f));
	SpawnTransform.SetRotation(RandomRotation);
	*/
	
	ACrop* NewCrop = GetWorld()->SpawnActorDeferred<ACrop>(
		ACrop::StaticClass(),
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
	if (NewCrop)
	{
		NewCrop->SetCropData(CropData);
		UGameplayStatics::FinishSpawningActor(NewCrop, SpawnTransform);
	}
}


// Called when the game starts or when spawned
void AACultivationPlot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AACultivationPlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

