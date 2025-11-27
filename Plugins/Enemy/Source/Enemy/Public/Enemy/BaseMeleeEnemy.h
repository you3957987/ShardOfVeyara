#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "BaseMeleeEnemy.generated.h"


UCLASS()
class ENEMY_API ABaseMeleeEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Attack() override;
};
