// Fill out your copyright notice in the Description page of Project Settings.


#include "ShardsAltar.h"

#include "Tribute.h"
#include "Components/SphereComponent.h"

// Sets default values
AShardsAltar::AShardsAltar()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	//루트 컴포넌트 설정
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AShardsAltar::OnBeginOverlap);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AShardsAltar::OnEndOverlap);
}

// Called when the game starts or when spawned
void AShardsAltar::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShardsAltar::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{		
		player->AddInteractableActor(this);
	}
}

void AShardsAltar::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

void AShardsAltar::Interact_Implementation(AAGSDCharacter* player)
{
}

void AShardsAltar::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool AShardsAltar::CanInteract_Implementation(AAGSDCharacter* player)
{
	return true;
}

