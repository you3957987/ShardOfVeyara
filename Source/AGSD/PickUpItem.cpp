// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpItem.h"
#include "Components/SphereComponent.h"
#include "AGSDPlayerController.h"

APickUpItem::APickUpItem()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetSimulatePhysics(true);

	Mesh->SetCollisionResponseToChannel(
	ECollisionChannel::ECC_Pawn,
	ECR_Overlap
	);
	Mesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECR_Overlap
		);
	Mesh->SetCollisionResponseToChannel(
			ECollisionChannel::ECC_PhysicsBody,  // 물리 시뮬레이션 중인 다른 액터
			ECollisionResponse::ECR_Ignore       // 충돌을 완전히 무시
		);

	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetSphereRadius(50);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APickUpItem::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &APickUpItem::OnEndOverlap);
}

void APickUpItem::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("PickUpItemOnBeginOverlap"));
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		IInteraction::Execute_ShowWidget(this, player);
		player->AddInteractableActor(this);

		InteractingPlayer = player;
	}
}

void APickUpItem::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);

		if (player == InteractingPlayer)
		{
			InteractingPlayer = nullptr;
		}
		if (player->GetInteractableActorNum() > 0) return;
		if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))	PlayerController->HideInteractionWidget();
	}
}

void APickUpItem::Interact_Implementation(AAGSDCharacter* player = nullptr)
{
	PickUpInteract(InteractingPlayer);
}

void APickUpItem::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

