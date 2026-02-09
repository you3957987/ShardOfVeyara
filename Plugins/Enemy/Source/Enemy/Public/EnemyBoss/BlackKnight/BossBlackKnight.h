#pragma once

#include "CoreMinimal.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "BossBlackKnight.generated.h"


USTRUCT(BlueprintType)
struct FBossBlackKnightAttackWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float RushAttack = 10.0f;
};

UCLASS()
class ENEMY_API ABossBlackKnight : public ABaseBossEnemy
{
	GENERATED_BODY()


protected:
	bool bIsRushing = false;        // 돌진 중인가?
	FVector RushTargetLocation;     // 목표 위치
	float RushDuration = 0.0f;      // 돌진 총 시간
	float RushElapsedTime = 0.0f;   // 경과 시간
	FVector RushStartLocation;      // 시작 위치
	// 돌진시 캐릭터 사이의 거리 계산해서 이동 시키는 함수
	void MoveForwardDuringRushAttack(float DeltaTime);

	
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


	// 돌진 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* RushAttackMontage;
	// 돌진 공격 함수
	void RushAttack();
	// 애님 노티파이에서 호출해서 돌진 시작할 타이밍 정하는 함수
	UFUNCTION(BlueprintCallable)
	void StartRush();

#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
