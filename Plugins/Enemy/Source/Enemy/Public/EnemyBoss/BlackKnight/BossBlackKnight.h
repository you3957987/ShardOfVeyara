#pragma once

#include "CoreMinimal.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "BossBlackKnight.generated.h"


USTRUCT(BlueprintType)
struct FBossBlackKnightAttackWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float HeavySlash = 10.0f;
};

UCLASS()
class ENEMY_API ABossBlackKnight : public ABaseBossEnemy
{
	GENERATED_BODY()


protected:
	

public:
	ABossBlackKnight();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FBossBlackKnightAttackWeight AttackWeight;

	// 이미 공격에 히트된 액터들을 저장하는 배열
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HittedActors;
	// 근접 공격 중인지 여부를 나타내는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	bool bIsAttacking = false; 

#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
