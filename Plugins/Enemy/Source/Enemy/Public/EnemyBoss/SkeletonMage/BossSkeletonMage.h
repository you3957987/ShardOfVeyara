#pragma once

#include "CoreMinimal.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "BossSkeletonMage.generated.h"

USTRUCT(BlueprintType)
struct FBossSkeletonMageAttackStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float TeleportWeight = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float TeleportDelay = 5.f;
	// 텔레포트 최대 거리
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MaxTeleportDist = 500.f;
	// 텔레포트 최소 거리
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MinTeleportDist = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float FireBallWeight = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float FireBallDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float SummonWeight = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float SummonEnemyDelay = 7.f;
	// 소환하는 씬 컴포넌트 기준 소환 가능한 최대 거리
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MaxSummonDist = 400.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float GroundAttackWeight = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float GroundAttackDelay = 4.f;
	// 장판 유지 시간
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float GroundAttackDuration = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float PushTargetWeight = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float PushTargetDelay = 3.f;
};

UCLASS()
class ENEMY_API ABossSkeletonMage : public ABaseBossEnemy
{
	GENERATED_BODY()

public:
	ABossSkeletonMage();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FBossSkeletonMageAttackStruct AttackStruct;
	
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UAnimMontage* TeleportMontage;
	void PlayTeleportMontage( const FVector& Destination );
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void TeleportMoveToNextPoint();
	// 텔레포트 목적지 위치 저장용 변수
	FVector TeleportDestination;
	// 텔레포트 시각 효과
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UNiagaraSystem* TeleportEffect;
	void SpawnTeleportEffectAtLocation(const FVector& Location);
	
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UAnimMontage* FireBallMontage;
	void PlayFireBallMontage();
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void ShootFireBall();
	// 파이어볼 발사체 클래스
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	TSubclassOf<class ABaseEnemyProjectile> FireBallClass;
	TObjectPtr<class USceneComponent> FireBallSpawnPoint;
	
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UAnimMontage* SummonEnemyMontage;
	// 소환 기준 씬 컴포넌트 1, 2
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SummonPointOne;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SummonPointTwo;
	void StartSummoning(const FVector& Location1, const FVector& Location2);
	UFUNCTION( BlueprintCallable )
	void SummonEnemy();
	// 소환할 적 클래스들의 배열. 0 미니언 1 로그 2 워리어
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	TArray<TSubclassOf<class ABaseEnemy>> SummonableEnemyClasses;
	// 소환될 위치들을 저장할 배열
	TArray<FVector> SummonLocations;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UNiagaraSystem* SummonEffectFromMage;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UNiagaraSystem* SummonEffectFromEnemy;
	void SpawnSummonEffectAtLocation(const FVector& Location);
	
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UAnimMontage* GroundAreaAttackMontage;
	void PlayGroundAreaAttackMontage();
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void GroundAreaAttack();
	// 장판 공격 발사체 클래스
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	TSubclassOf<class AGroundAttackProjectile> GroundAttackClass;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UNiagaraSystem* GroundTargetingEffect;
	/** 스폰된 GroundTargetingEffect 컴포넌트를 추적하기 위한 포인터 */
	UPROPERTY()
	class UNiagaraComponent* GroundTargetingComponent;
	void TraceTargetCharacterForGroundAttackEffect(float DeltaTime);
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UNiagaraSystem* GroundAttackEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UAnimMontage* PushTargetMontage;
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HittedActors;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	bool bIsAttacking = false; 
	// 푸시 범위 원형 콜리전 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* PushAreaSphere;
	void PlayPushTargetMontage();
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void PushTarget_Start( ){ HittedActors.Empty();bIsAttacking = true; };
	UFUNCTION( BlueprintCallable )
	void PushTarget_End( ){HittedActors.Empty();bIsAttacking = false;} ;
	UFUNCTION( BlueprintCallable )
	void CreateMagicShield();
	// 수평으로 밀어내는 힘
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	float PushForce = 2000.f;
	// 위로 띄우는 힘
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	float PushUpwardForce = 400.f; 
	// 쉴드 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UNiagaraSystem* MagicShieldEffect;
};
