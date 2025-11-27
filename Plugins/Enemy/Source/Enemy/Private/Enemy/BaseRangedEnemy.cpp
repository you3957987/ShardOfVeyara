#include "Enemy/BaseRangedEnemy.h"

void ABaseRangedEnemy::Attack()
{
	Super::Attack();

	//UE_LOG(LogTemp, Warning, TEXT("RangedEnemyAttackCall") );
}

// 발사체 발사 함수
void ABaseRangedEnemy::ShootProjectile()
{
	Super::ShootProjectile();// 기본 동작 호출
}
