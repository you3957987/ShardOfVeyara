#pragma once

#include "CoreMinimal.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "BossMagicSwordMan.generated.h"

USTRUCT(BlueprintType)
struct FBossMagicSwordManAttackWeight
{
	GENERATED_BODY()
	
	// 근점 공격 - 3개중 랜덤
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float CloseAttack = 10.0f;
	
};
UCLASS()
class ENEMY_API ABossMagicSwordMan : public ABaseBossEnemy
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* WeaponCollision = nullptr;
	UPROPERTY(VisibleAnywhere)
	class UMotionWarpingComponent* MotionWarpingComponent;
	
	float AttackDamage;
	
public:
	ABossMagicSwordMan();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION()
	void OnBeginOverlapWeaponCollisionSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable)
	void AttackStart_WeaponCollision();
	UFUNCTION(BlueprintCallable)
	void AttackEnd_WeaponCollision();
	
	// 모션 워핑할 타깃 캐릭터 좌표 찍기
	UFUNCTION(BlueprintCallable)
	void UpdateMotionWarpTarget();
	// 애님 노티파이에서 공격 대미지 설정
	UFUNCTION(BlueprintCallable)
	void SetAttackDamage(float DamageToApply);
	// 공격 가중치 구조체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FBossMagicSwordManAttackWeight AttackWeight;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsGuarding = false;
	float DamageWhileGuarding = 0.f;
	// 가드 유지 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float GuardDuration = 5.f;
	// 반격 발동에 필요한 최대 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float MaxDamageToReaction = 10.f;
	// 가드시 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* GuardMontage;
	// 단순 가드 패턴 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float GuardDelay = 4.0f;

	// 근점 공격 시작 함수 - 랜덤 3개. 선택된 몽타주 반환
	UAnimMontage* StartCloseAttack();
	// 근접 공격 함수 배열
	UPROPERTY(EditAnywhere, Category = "자체설정")
	TArray<UAnimMontage*> CloseAttackMontages;
	

	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
