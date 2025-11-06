#include "Enemy/BaseMeleeEnemy.h"

void ABaseMeleeEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ABaseMeleeEnemy::Attack()
{
	Super::Attack();
	//UE_LOG(LogTemp, Warning, TEXT("MeleeEnemyAttackCall") );
}
