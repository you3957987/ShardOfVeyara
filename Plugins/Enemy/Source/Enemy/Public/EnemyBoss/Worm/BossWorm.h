#pragma once

#include "CoreMinimal.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "BossWorm.generated.h"

USTRUCT(BlueprintType)
struct FBossWormAttackWeight
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float NormalAttack = 5.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float Burrow = 3.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float Unburrow = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float RangedAttack = 4.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float LungeAttack = 3.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float LinearFireBreath = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float FanFireBreath = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float SuctionAttack = 3.0f;
};

UCLASS()
class ENEMY_API ABossWorm : public ABaseBossEnemy
{
	GENERATED_BODY()
	
protected:
	class USphereComponent* AttackCollisionSphere = nullptr;
	class USceneComponent* FireBreathPoint = nullptr;
	
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* UnBurrowAttackCollisionSphere;
	
	float AttackDamage = 20.f;
	
public:
	ABossWorm();
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
	
	// 공격 가중치 구조체 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FBossWormAttackWeight AttackWeight;
	
	// 땅파는 중인지 여부를 나타내는 변수 + 블루프린트 에서 읽기 전용으로 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "자체설정")
	bool bIsBurrowing = false; 
	
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
	float NormalAttackDelay = 3.0f;
	
	// 런지 공격 함수
	void LungeAttack();
	// 런지 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* LungeAttackMontage;
	// 런지 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float LungeAttackDamage = 35.f;
	// 런지 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float LungeAttackDelay = 5.0f;
	
	// 버로우 함수
	void Burrow();
	// 애님 노티파이 함수 - 땅 아래로 들어가는 몽타주 끝나는 시점에 호출되어 땅파는 상태를 true로 변경
	UFUNCTION(BlueprintCallable)
	void FinishBurrow(); 
	// 땅 아래로 들어가는 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* BurrowMontage; 
	// 버로우 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float BurrowDelay = 5.0f;
	// 언버로우 타깃 위치 세팅하는 함수 == BurrowDelay 에서 2초 정도 남았을 때 호출되어 타겟 위치 세팅
	FTimerHandle UnBurrowTargetTimerHandle; 
	void SetUnBurrowTargetLocation();
	
	// 언버로우
	void Unburrow();
	// 애님 노티파이 함수 - 땅에서 나오는 몽타주 끝나는 시점에 호출되어 땅파는 상태를 false로 변경
	UFUNCTION(BlueprintCallable)
	void FinishUnburrow();
	// 땅에서 나오는 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* UnburrowMontage;
	// 언버로우 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float UnBurrowAttackDamage = 30.f;
	// 언버로우 공격 어택 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float UnBurrowAttackDelay = 3.0f;
	// 언버로우시 이동할 위치
	FVector UnBurrowTargetLocation;
	
	// 원거리 공격 타입일 때 발사체 클래스
	UPROPERTY(EditAnywhere, Category="자체설정")
	TSubclassOf<class ABaseEnemyProjectile> RangedProjectileClass;
	// 원거리 공격 지점 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* RangedAttackPoint;
	// 원거리 공격 함수
	void RangedAttack();
	// 발사체 날아가는 애님 노티파이 함수
	UFUNCTION(BlueprintCallable)
	void ShootRangedProjectile();
	// 원거리 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* RangedAttackMontage;
	// 원거리 공격 딜레이 
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float RangedAttackDelay = 4.0f;

	// 화염 방사 시작 함수
	UFUNCTION(BlueprintCallable)
	void StartFireBreath();
	// 화염 방사 종료 함수
	UFUNCTION(BlueprintCallable)	
	void EndFireBreath();
	// 화염 방사 발사체
	UPROPERTY(EditAnywhere, Category="자체설정")
	TSubclassOf<class ABaseStreamProjectile> StreamProjectileClass;
	// 연사 타이머 핸들
	FTimerHandle FireBreathTimerHandle; 
	void SpawnFireBreathProjectile(); 
	// 화염 방사 간격
	UPROPERTY(EditAnywhere, Category = "자체설정")	
	float FireBreathInterval = 0.05f;
	
	// 직선 화염 방사 함수
	void LinearFireBreathStart();
	// 직선 화염 방사 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")
	UAnimMontage* LinearFireBreathMontage;
	// 직선 화염 방사 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")	
	float LinearFireBreathDelay = 5.0f;
	// 직선 화염 방사 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float LinearFireBreathDamage = 4.f;
	
	// 부채꼴 화염 방사 함수
	void FanFireBreathStart();
	// 부채꼴 화염 방사 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")	
	UAnimMontage* FanFireBreathMontage;
	// 부채꼴 화염 방사 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float FanFireBreathDelay = 5.0f;
	// 부채꼴 화염 방사 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	float FanFireBreathDamage = 3.f;
	
	// 빨아들이기 로직
	void HandleSuction(float DeltaTime);
	// 빨아들이는 공격 함수
	void SuctionStartMontagePlay();
	// 빨아들이는 공격 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "자체설정")	
	UAnimMontage* SuctionMontage;
	// 빨아들이고 난 후 거리 내 공격 대미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")	
	float SuctionAttackDamage = 40.f;
	// 빨아들이는 공격 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float SuctionAttackDelay = 4.0f;
	// 빨아들이는 힘
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float SuctionForce = -700000.f;
	// 빨아들이기 시작 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void StartSuction();
	// 빨아들이기 끝 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)	
	void EndSuction();
	// 빨아들이다가 거리 안에 캐릭터 있는지에 대한 플래그
	bool bIsInSuctionAttackArea = false;
	// 빨아들이는 중인지 여부를 나타내는 변수
	bool bIsSuctioning = false; 
	// radial force 컴포넌트로 빨아들이는 로직 만들기
	UPROPERTY(VisibleAnywhere)
	class URadialForceComponent* SuctionRadialForceComp;
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	
};
