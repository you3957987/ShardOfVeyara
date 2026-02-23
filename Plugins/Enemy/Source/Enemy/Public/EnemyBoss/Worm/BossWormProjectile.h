#pragma once

#include "CoreMinimal.h"
#include "EnemyProjectile/BaseEnemyProjectile.h"
#include "BossWormProjectile.generated.h"

UCLASS()
class ENEMY_API ABossWormProjectile : public ABaseEnemyProjectile
{
	GENERATED_BODY()
	
protected:
	virtual void CreateHitEffect() override;
public:
	ABossWormProjectile();
};
