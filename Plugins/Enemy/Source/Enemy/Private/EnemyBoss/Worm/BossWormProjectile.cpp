#include "EnemyBoss/Worm/BossWormProjectile.h"

#include "NiagaraFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABossWormProjectile::ABossWormProjectile()
{
	bOnlyNiagaraEffect = false;
	
	// 중력 영향 받도록 설정
	ProjectileMovement->ProjectileGravityScale = 1.0f;
}

void ABossWormProjectile::CreateHitEffect()
{
	/*
	if (HitEffect && GetWorld() && EffectCreateLocation != FVector::ZeroVector)
	{
		// 시계 방향으로 90도 회전한 값으로 이펙트를 생성합니다.
		const FRotator EffectRotation(90.f, 0.f, 0.f);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, EffectCreateLocation, EffectRotation);
	}
	*/
	
	Super::Destroyed();
}