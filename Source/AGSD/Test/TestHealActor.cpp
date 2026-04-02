#include "TestHealActor.h"

#include "AGSDCharacter.h"
#include "Components/SphereComponent.h"

ATestHealActor::ATestHealActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetVisibility(true);
	// 게임에서 숨김 false로 
	CollisionSphere->SetHiddenInGame(false);
	
}

void ATestHealActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (CollisionSphere)
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATestHealActor::OnOverlapBegin);
	}
}

void ATestHealActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATestHealActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		if (OtherActor && OtherActor->ActorHasTag("Player"))
		{
			// AAGSDCharacter로 캐스팅하여 회복 함수 호출
			if (auto* Player = Cast<AAGSDCharacter>(OtherActor))
			{
				Player->HealthRecovery(500.0f); // 20만큼 회복
			}
		}
	}
}

