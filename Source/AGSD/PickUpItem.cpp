// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpItem.h"
#include "Components/SphereComponent.h"
#include "AGSDPlayerController.h"
#include "AGSDCharacter.h"

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

void APickUpItem::BeginPlay()
{
	Super::BeginPlay();
	if (Holding)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void APickUpItem::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("PickUpItemOnBeginOverlap"));
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->AddInteractableActor(this);
	}
}

void APickUpItem::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

void APickUpItem::Interact_Implementation(AAGSDCharacter* player)
{
	PickUpInteract(player);
}

void APickUpItem::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool APickUpItem::CanInteract_Implementation(AAGSDCharacter* player)
{
	return true;
}

