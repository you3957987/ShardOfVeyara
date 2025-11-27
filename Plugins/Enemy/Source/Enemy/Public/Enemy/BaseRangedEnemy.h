#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "BaseRangedEnemy.generated.h"


UCLASS()
class ENEMY_API ABaseRangedEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	virtual void Attack() override;
	virtual void ShootProjectile() override;
};
