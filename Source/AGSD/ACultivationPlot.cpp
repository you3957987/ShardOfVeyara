// Fill out your copyright notice in the Description page of Project Settings.


#include "ACultivationPlot.h"
#include "Components/BoxComponent.h"
#include "AGSDCharacter.h"
#include "Crop.h"

#include "MovieSceneSequenceID.h"

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
	CollisionBox->SetBoxExtent(FVector(60.0f, 60.f, 60.f));
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AACultivationPlot::OnBeginOverlap);
}

//오버랩 시작 함수 구현부
void AACultivationPlot::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AAGSDCharacter>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("AACultivationPlot::OnBeginOverlap"));
		ACrop* NewCrop = GetWorld()->SpawnActorDeferred<ACrop>(
			ACrop::StaticClass(),
			GetTransform(),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);
		if (NewCrop)
		{
			NewCrop->SetCropData(CropData);
			NewCrop->FinishSpawning(GetTransform());
		}
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

