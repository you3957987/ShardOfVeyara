#pragma once

#include "CoreMinimal.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "BossSkeletonMage.generated.h"

USTRUCT(BlueprintType)
struct FBossSkeletonMageAttackWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float Teleport = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float FireBall = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float Summon = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float GroundAttack = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float PushTarget = 3.0f;
};

UCLASS()
class ENEMY_API ABossSkeletonMage : public ABaseBossEnemy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float TeleportDelay = 10.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* TeleportMontage;
	// 텔레포트 목적지 위치 저장용 변수
	FVector TeleportDestination;
	// 텔레포트 시각 효과
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* TeleportEffect;
	void SpawnTeleportEffectAtLocation(const FVector& Location);
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float FireBallDelay = 5.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireBallMontage;
	// 파이어볼 발사체 클래스
	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class ABaseEnemyProjectile> FireBallClass;
	TObjectPtr<class USceneComponent> FireBallSpawnPoint;

	UPROPERTY( EditAnywhere, Category = "자체설정" )
	float SummonEnemyDelay = 12.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* SummonEnemyMontage;
	// 소환할 적 클래스들의 배열. 0 미니언 1 로그 2 워리어
	UPROPERTY(EditAnywhere, Category = "자체설정")
	TArray<TSubclassOf<class ABaseEnemy>> SummonableEnemyClasses;
	// 소환될 위치들을 저장할 배열
	TArray<FVector> SummonLocations;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* SummonEffectFromMage;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* SummonEffectFromEnemy;
	void SpawnSummonEffectAtLocation(const FVector& Location);
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float GroundAttackDelay = 8.f;
	// 장판 유지 시간
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float GroundAttackDuration = 3.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* GroundAreaAttackMontage;
	// 장판 공격 발사체 클래스
	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class AGroundAttackProjectile> GroundAttackClass;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* GroundTargetingEffect;
	/** 스폰된 GroundTargetingEffect 컴포넌트를 추적하기 위한 포인터 */
	UPROPERTY()
	class UNiagaraComponent* GroundTargetingComponent;
	void TraceTargetCharacterForGroundAttackEffect(float DeltaTime);
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* GroundAttackEffect;
	
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float PushTargetDelay = 3.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* PushTargetMontage;
	// 수평으로 밀어내는 힘
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float PushForce = 2000.f;
	// 위로 띄우는 힘
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float PushUpwardForce = 400.f; 
	// 쉴드 이펙트
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* MagicShieldEffect;

public:
	ABossSkeletonMage();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FBossSkeletonMageAttackWeight AttackWeight;
	
	// 텔레포트 최대 거리
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MaxTeleportDist = 500.f;
	// 텔레포트 최소 거리
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MinTeleportDist = 300.f;
	void PlayTeleportMontage( const FVector& Destination );
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void TeleportMoveToNextPoint();

	void PlayFireBallmontage();
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void ShootFireBall();
	// 소환하는 씬 컴포넌트 기준 소환 가능한 최대 거리
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MaxSummonDist = 600.f;
	// 소환 기준 씬 컴포넌트 1, 2
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SummonPointOne;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SummonPointTwo;
	
	void StartSummoning(const FVector& Location1, const FVector& Location2);
	UFUNCTION( BlueprintCallable )
	void SummonEnemy();

	void PlayGroundAreaAttackMontage();
	// 애님 노티파이에서 호출할 함수
	UFUNCTION( BlueprintCallable )
	void GroundAreaAttack();
	
	// 이미 공격에 히트된 액터들을 저장하는 배열
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HittedActors;
	// 근접 공격 중인지 여부를 나타내는 변수
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
};
