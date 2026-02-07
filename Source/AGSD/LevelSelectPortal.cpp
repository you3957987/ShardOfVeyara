// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelSelectPortal.h"

#include "Components/BoxComponent.h"

// Sets default values
ALevelSelectPortal::ALevelSelectPortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Portal"));
	RootComponent = CollisionBox;

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ALevelSelectPortal::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ALevelSelectPortal::OnEndOverlap);
}

// Called when the game starts or when spawned
void ALevelSelectPortal::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALevelSelectPortal::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void ALevelSelectPortal::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

