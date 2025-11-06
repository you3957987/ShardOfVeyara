#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

UENUM(BlueprintType, Meta=(DisplayName="Enemy Type")) // 적 타입을 정의하는 열거형
enum class EEnemyType : uint8
{
	EET_Melee UMETA(DisplayName = "Melee Enemy"), // 근접 공격 적
	EET_Ranged UMETA(DisplayName = "Ranged Enemy"), // 원거리 공격 적

	EET_MAX UMETA(DisplayName = "Default") // 최대값, 추가적인 값을 위한 공간
};

UCLASS()
class ENEMY_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

	friend class AEnemyAiController; // EnemyAiController에서 BaseEnemy의 protected 멤버에 접근할 수 있도록 합니다.
	friend class UBTTask_BaseMeleeAttack; // BTTask_BaseMeleeAttack에서 BaseEnemy의 protected 멤버에 접근할 수 있도록 합니다.
	friend class UBTTask_BaseRangedAttack; // BTTask_BaseRangedAttack에서 BaseEnemy의 protected 멤버에 접근할 수 있도록 합니다.
	
protected:
	virtual void BeginPlay() override;

	void PollInit(); // 틱에서 하는 초기화
	
	// 적 타입
	UPROPERTY(EditAnywhere, Category="자체설정")
	EEnemyType EnemyType = EEnemyType::EET_Melee;
	// 원거리 공격 타입일 때 발사체 클래스
	UPROPERTY(EditAnywhere, Category="자체설정")
	TSubclassOf<class ABaseEnemyProjectile> ProjectileClass;
	// 최대 체력
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MaxHealth = 100.f;
	// 현재 체력
	UPROPERTY(EditAnywhere, Category="자체설정")
	float Health = 100.f;
	// 이동 속도 = 여기서 값 바꾸면 자동으로 캐릭터 무브먼트 컴포넌트의 MaxWalkSpeed도 바뀜
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MoveSpeed = 300.f;
	// 이거 수정하면 자동으로 AttackRangeSphere의 반지름도 수정되게 만듬
	UPROPERTY(EditAnywhere, Category="자체설정")
	float AttackRange = 100.f;
	// AI가 플레이어를 인식하는 범위
	UPROPERTY(EditAnywhere, Category="자체설정")
	float DetectRange = 700.f; 
	// AI가 플레이어를 추적하는 범위
	UPROPERTY(EditAnywhere, Category="자체설정")
	float ChaseRange = 1000.f; 
	// 공격 딜레이
	UPROPERTY(EditAnywhere, Category="자체설정")
	float AttackDelay = 1.5f;
	// 패트롤 딜레이
	UPROPERTY(EditAnywhere, Category="자체설정")
	float PatrolDelay = 3.f;
	// 근접 공격 데미지
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MeleeAttackDamage = 20.f; // 
	
	
	// 타겟으로 삼을 캐릭터를 저장할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<ACharacter> TargetCharacter;
	// 캐릭터 초기화 여부를 나타내는 변수
	bool bTargetInitalize = false;
	// 충돌 구 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AttackRangeSphere; 
	// 인식 범위 확인용 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* DetectRangeSphere; 
	// 추적 범위 확인용 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* ChaseRangeSphere;;
	// 근접 공격 지점 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* MeleeAttackPoint;
	// 근접 공격 범위
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AttackRangePointSphere;
	// 원거리 공격 지점 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* RangedAttackPoint;

	// 체력 바 위젯 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<class UWidgetComponent> HealthBarWidget;
	// Tick에서 체력 바 위젯을 내 캐릭터 쪽으로 돌아보게 하는 함수
	void UpdateHealthBarWidget(float DeltaTime);
	
	// 공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* AttackMontage; 
	// 죽음 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* DeathMontage;
	// 스폰시 재생할 몽타주
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* SpawnMontage;

public:
	ABaseEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// 데미지 처리 함수 재정의
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsDead() const { return Health <= 0.f; }; // 체력이 0 이하이면 죽은 상태로 간주합니다.
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsSpawnEnd() const { return !bUseSpawnMontage; }; // 스폰 몽타주 끝났는지 여부 반환
	void Die(); // 죽음 처리 함수
	
	// 스폰 효과 사용 여부. 기본은 트루이지만 스폰 몽타주 안넣으면 flase랑 동일
	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bUseSpawnMontage = true;
	// 스폰 몽타주 끝난 후 AI 컨트롤러 빙의 함수 - 애님 노티파이에서 호출
	UFUNCTION( BlueprintCallable )
	void SpawnAndPossessAIController();
	// 스폰 몽타주 초반에 메시를 보이게 하는 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void ShowCharacterMesh();
	
	// 무조건 플레이어 추적 모드
	UPROPERTY(EditAnywhere, Category="자체설정")
	bool bAlwaysChase = false;
	// 이미 공격에 히트된 액터들을 저장하는 배열
	UPROPERTY()
	TArray<TObjectPtr<AActor>> HittedActors;
	// 근접 공격 중인지 여부를 나타내는 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	bool bIsAttacking = false; 
	// 근접 공격 범위 활성화 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void AttackCheck_Start() { HittedActors.Empty();bIsAttacking = true;} ;
	// 근접 공격 범위 비활성화 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void AttackCheck_End() { HittedActors.Empty();bIsAttacking = false; };
	// 원거리 발사체 발사 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	virtual void ShootProjectile();
	
	// ATargetPoint 액터를 패트롤 지점으로 사용하기 위한 새 프로퍼티
	UPROPERTY(EditAnywhere, Category = "자체설정", meta=(DisplayName="Patrol Points"))
	TArray<TObjectPtr<class ATargetPoint>> PatrolPoints;

	
	virtual void Attack(); // 공격 함수. 

#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
