#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "BaseBurrowEnemy.generated.h"


UCLASS()
class ENEMY_API ABaseBurrowEnemy : public ABaseEnemy
{
	GENERATED_BODY()
	
protected:
	
	// 튀어나올시 공격
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* BurrowAttackCollisionSphere;
	// 근접 공격 지점 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* MeleeAttackPoint;
	// 근접 공격 범위
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AttackRangePointSphere;
	
	float AttackDamage = 20.f;
	
public:
	ABaseBurrowEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, 
		struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	UFUNCTION()
	void OnBeginOverlapAttackCollisionSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void AttackStart_AttackCollisionSphere();
	UFUNCTION(BlueprintCallable)
	void AttackEnd_AttackCollisionSphere();
	
	// 땅파는 중인지 여부를 나타내는 변수 + 블루프린트 에서 읽기 전용으로 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	bool bIsBurrowing = true; 

	// 땅 아래로 들어가는 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* BurrowMontage; 
	// 땅에서 나오는 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* UnburrowMontage; 
	// 땅 아래로 들어가는 몽타주 재생 함수
	void PlayBurrowMontage();
	// 땅에서 나오는 몽타주 재생 함수
	void PlayUnburrowMontage();
	// 애님 노티파이 함수 - 땅에서 나오는 몽타주 끝나는 시점에 호출되어 땅파는 상태를 false로 변경
	UFUNCTION(BlueprintCallable)
	void FinishUnburrow();
	// 애님 노티파이 함수 - 땅 아래로 들어가는 몽타주 끝나는 시점에 호출되어 땅파는 상태를 true로 변경
	UFUNCTION(BlueprintCallable)
	void FinishBurrow();

	// 탐지 범위 스피어 비긴 오버랩 함수
	UFUNCTION()
	void OnBeginOverlapDetectRangeSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// 탐지 범위 스피어 엔드 오버랩 함수
	UFUNCTION()
	void OnEndOverlapChaseRangeSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
