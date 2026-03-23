#include "EnemyProjectile/BaseDelayedBurstProjectile.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseDelayedBurstProjectile::ABaseDelayedBurstProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
}

void ABaseDelayedBurstProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 1. 폭발 전조 이펙트 재생 (지속형)
	if (PreExplosionEffect)
	{
		PreExplosionComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			PreExplosionEffect,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// 2. 일정 시간(DelayBeforeBurst) 뒤에 폭발 함수 실행
	GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &ABaseDelayedBurstProjectile::Explode, DelayBeforeBurst, false);
}

void ABaseDelayedBurstProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseDelayedBurstProjectile::Explode()
{
	// 1. 전조 이펙트 제거
	if (PreExplosionComp)
	{
		PreExplosionComp->Deactivate(); // 파티클 생성 중단
		PreExplosionComp->DestroyComponent();
		PreExplosionComp = nullptr;
	}

	// 2. 폭발 이펙트 재생 (일회성)
	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	// 3. 범위 내 대상 감지 및 대미지 처리
	TArray<AActor*> OverlappingActors;
	if (DamageSphere)
	{
		DamageSphere->GetOverlappingActors(OverlappingActors);
	}

	AController* InstigatorController = GetInstigatorController();

	for (AActor* Actor : OverlappingActors)
	{
		// 자기 자신 무시, 플레이어 태그 확인 (필요시 태그 조건 제거 가능)
		if (Actor && Actor != this && Actor->ActorHasTag("Player"))
		{
			// 대미지 적용
			UGameplayStatics::ApplyDamage(
				Actor,
				ExplosionDamage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
	}

	// 4. 투사체(자신) 파괴
	Destroy();
}
