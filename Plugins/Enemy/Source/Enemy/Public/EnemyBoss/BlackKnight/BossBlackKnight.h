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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float Guard = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float NormalAttack = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float ChargeAttack = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float ZapAttack = 7.0f;
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
	void MoveForwardDuringRushAttack(float DeltaTime);
	
	class USphereComponent* AxeCollisionSphere = nullptr;

	// 공격시 줄 대미지 == 공격 전에 함수에서 각각 변경
	float AttackDamage = 0.f;
	
public:
	ABossBlackKnight();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FBossBlackKnightAttackWeight AttackWeight;
	
	// 도끼 공격 범위에 플레이어가 들어왔을 때 호출되는 함수
	UFUNCTION()
	void OnBeginOverlapAxeCollisionSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void AttackStart_AxeCollisionSphere();
	UFUNCTION(BlueprintCallable)
	void AttackEnd_AxeCollisionSphere();

	
	// 돌진 공격 범위
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* RushAttackSphere;
	// 돌진 공격 범위에 플레이어가 들어왔을 때 호출되는 함수
	UFUNCTION()
	void OnBeginOverlapRushAttackSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// 돌진 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float RushAttackDamage = 30.f;
	// 수평으로 밀어내는 힘
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float PushForce = 1500.f;
	// 위로 띄우는 힘
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float PushUpwardForce = 200.f;
	// 돌진 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* RushAttackMontage;
	// 돌진 공격 함수
	void RushAttack();
	// 돌진 전 캐릭 위치 세팅
	UFUNCTION(BlueprintCallable)
	void SetRushTargetLocation();
	// 애님 노티파이에서 호출해서 돌진 시작할 타이밍 정하는 함수
	UFUNCTION(BlueprintCallable)
	void StartRush();
	// 돌진 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float RushAttackDelay = 3.0f;
	
	// ABP에서 가드중인지 설정하는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
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
	
	// 가드 공격 함수
	void GuardAttack();
	// 가드 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* GuardAttackMontage;
	// 가드 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float GuardAttackDamage = 25.f;
	// 번개 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float ZapDamage = 15.f;
	bool bIsHitZap = false;
	// 가드 공격시 나갈 나이라가라 이펙트
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* ZapEffect;
	// 방사형 번개 공격 애님 노티파이 함수
	UFUNCTION(BlueprintCallable)
	void StartWaveAttack();
	// 가드 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float GuardAttackDelay = 2.0f;
	
	// 기본 공격 함수
	void NormalAttack();
	// 기본 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* NormalAttackMontage;
	// 기본 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float NormalAttackDamage = 25.f;
	// 기본 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float NormalAttackDelay = 2.0f;
	
	// 차지 공격 함수
	void ChargeAttack();
	// 차지 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* ChargeAttackMontage;
	// 차지 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float ChargeAttackDamage = 35.f;
	// 차지 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ChargeAttackDelay = 3.0f;
	// 랜덤하게 번개 이펙트 생성 함수
	UFUNCTION(BlueprintCallable)
	void SpawnRandomZapEffect();
	
	// 잽 번개 공격 함수
	void ZapAttack();
	// 잽 번개 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")	
	UAnimMontage* ZapAttackMontage;
	// 잽 번개 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float ZapAttackDamage = 20.f;
	// 잽 번개 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ZapAttackDelay = 2.5f;
	// 잽 공격 좌표 설정
	UFUNCTION(BlueprintCallable)
	void SetZapTargetLocation();
	FVector ZapTargetLocation; // 잽 공격 목표 위치 저장 변수
	// 잽 생성 타이밍
	UFUNCTION(BlueprintCallable)
	void SpawnZapAttackEffect();
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
