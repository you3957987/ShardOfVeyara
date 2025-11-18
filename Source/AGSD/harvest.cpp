// Fill out your copyright notice in the Description page of Project Settings.


#include "harvest.h"
#include "Components/sphereComponent.h"
#include "AGSDCharacter.h"
#include "AGSDPlayerController.h"

// Sets default values
Aharvest::Aharvest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//루트 컴포넌트 설정
	HarvestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HarvestMesh"));
	RootComponent = HarvestMesh;
	HarvestMesh->SetSimulatePhysics(true);

	HarvestMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECR_Overlap
		);
	HarvestMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECR_Overlap
		);
	HarvestMesh->SetCollisionResponseToChannel(
			ECollisionChannel::ECC_PhysicsBody,  // 물리 시뮬레이션 중인 다른 액터
			ECollisionResponse::ECR_Ignore       // 충돌을 완전히 무시
		);

	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetSphereRadius(75);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &Aharvest::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &Aharvest::OnEndOverlap);
}

void Aharvest::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->AddInteractableActor(this);
	}
}

void Aharvest::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

void Aharvest::Interact_Implementation(AAGSDCharacter* player)
{
	UE_LOG(LogTemp, Display, TEXT("Interacting with %s"), *GetNameSafe(player));
	HarvestInteract(player);
}

void Aharvest::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool Aharvest::CanInteract_Implementation(EHoldingState state)
{
	return true;
}
