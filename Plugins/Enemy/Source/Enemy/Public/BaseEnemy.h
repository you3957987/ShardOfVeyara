#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

// 에너미 폴더 안의 ENUM 안에다가도 추가해줘야함!!!!!!!!!!!!!!!!!!!!!!!!
// 에너미 폴더 안의 ENUM 안에다가도 추가해줘야함!!!!!!!!!!!!!!!!!!!!!!!!
// 에너미 폴더 안의 ENUM 안에다가도 추가해줘야함!!!!!!!!!!!!!!!!!!!!!!!!
UENUM(BlueprintType, Meta=(DisplayName="Enemy Type")) // 적 타입을 정의하는 열거형
enum class EEnemyType : uint8
{
	EET_Melee UMETA(DisplayName = "Melee Enemy"), // 근접 공격 적
	EET_Ranged UMETA(DisplayName = "Ranged Enemy"), // 원거리 공격 적
	EET_Exploder UMETA(DisplayName = "Exploder Enemy"), // 폭발 적
	EET_Transpar UMETA(DisplayName = "Transpar Enemy"), // 투명 몹
	EET_Mimic UMETA(DisplayName = "Mimic Enemy"), // 위장 몹
	EET_Slime UMETA(DisplayName = "Slime Enemy"), // 슬라임 몹(분열)
	EET_Mage UMETA(DisplayName = "Mage Enemy"), // 마법사 몹 - 일단 안에 가능한 모든 공격 만들고 애님 노티파이로 결정

	EET_MAX UMETA(DisplayName = "Default") // 최대값, 추가적인 값을 위한 공간
};

UENUM(BlueprintType, Meta=(DisplayName="Enemy Type")) // 적 타입을 정의하는 열거형
enum class EEnemyState : uint8
{
	EES_Patrol UMETA(DisplayName = "Patrol"), // 패트롤 상태
	EES_Chase UMETA(DisplayName = "Chase"), // 추적 상태
	EES_Attack UMETA(DisplayName = "Attack"), // 공격 상태
	EES_Dead UMETA(DisplayName = "Dead"), // 죽음 상태

	EES_MAX UMETA(DisplayName = "Default") // 최대값, 추가적인 값을 위한 공간
};

UCLASS()
class ENEMY_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

	friend class AEnemyAiController; // EnemyAiController에서 BaseEnemy의 protected 멤버에 접근할 수 있도록 합니다.

protected:
	virtual void BeginPlay() override;

	void PollInit(); // 틱에서 하는 초기화

	// 디버그 모드
	UPROPERTY(EditAnywhere, Category="자체설정")
	bool bDebugMode = false;
	// 죽음 로직 체크. 3초 뒤에 체력을 0으로 만든 후 Die() 호출
	UPROPERTY(EditAnywhere, Category="자체설정")
	bool bCheckDeadLogic = false;
	void TestDeadLogic(); // 테스트용 죽음 로직 함수
	// 적 타입
	UPROPERTY(EditAnywhere, Category="자체설정")
	EEnemyType EnemyType = EEnemyType::EET_Melee;
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
	
	// 체력 바 위젯 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<class UWidgetComponent> HealthBarWidget;
	// Tick에서 체력 바 위젯을 내 캐릭터 쪽으로 돌아보게 하는 함수
	void UpdateHealthBarWidget(float DeltaTime);
	
	// 죽음 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* DeathMontage;
	// 스폰시 재생할 몽타주
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* SpawnMontage;

public:
	ABaseEnemy();
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsSpawnEnd() const { return !bUseSpawnMontage; }; // 스폰 몽타주 끝났는지 여부 반환

	// 데미지 처리 함수 재정의
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// 현재 적 상태 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	EEnemyState CurrentState = EEnemyState::EES_Patrol; 
	// 적 상태 설정 함수
	UFUNCTION(BlueprintCallable)
	void SetEnemyState(EEnemyState NewState) { CurrentState = NewState; }; 

	// 죽을 떄 생성할 이펙트 == 케스케이드
	UPROPERTY(EditAnywhere, Category="자체설정")
	class UParticleSystem* DeathEffectCascade;
	// 죽음 이펙트 생성 앞 뒤 위치 조정용 거리
	UPROPERTY(EditAnywhere, Category="자체설정")
	float DeathEffectForwardOffset = 0.0f;
	// 죽음 이펙트 생성 위 아래 위치 조정용 거리
	UPROPERTY(EditAnywhere, Category="자체설정")
	float DeathEffectUpOffset = 0.0f;
	// 죽음 이펙트 크기 배율
	UPROPERTY(EditAnywhere, Category="자체설정")
	float DeathEffectScale = 1.0f;
	// 죽음 처리 함수
	void Die();
	// 죽음 몽타주 끝난 후 호출되는 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	virtual void AfterDieMontageEnd(); 
	// 죽고 나서 떨어질 아이템 배열
	UPROPERTY(EditAnywhere, Category="자체설정")
	TArray<TSubclassOf<AActor>> DropItems;
	// 아이템 드롭 함수
	void DropItemsAfterDead();
	
	// 죽음 후 일정 시간 뒤에 이펙트 생성 및 액터 제거를 위한 타이머 핸들
	FTimerHandle DeathTimerHandle;
	virtual void SpawnDeadEffectAndDestroy();
	
	// 스폰 효과 사용 여부. 기본은 트루이지만 스폰 몽타주 안넣으면 false랑 동일
	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bUseSpawnMontage = false;
	// 스폰 몽타주 끝난 후 AI 컨트롤러 빙의 함수 - 애님 노티파이에서 호출
	UFUNCTION( BlueprintCallable )
	void SpawnAndPossessAIController();
	// 스폰 몽타주 초반에 메시를 보이게 하는 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void ShowCharacterMesh();
	
	// 무조건 플레이어 추적 모드
	UPROPERTY(EditAnywhere, Category="자체설정")
	bool bAlwaysChase = false;
	
	// ATargetPoint 액터를 패트롤 지점으로 사용하기 위한 새 프로퍼티
	UPROPERTY(EditAnywhere, Category = "자체설정", meta=(DisplayName="Patrol Points"))
	TArray<TObjectPtr<class ATargetPoint>> PatrolPoints;
	
	virtual void Attack(); // 공격 함수. 
	// 공격 애니메이션 몽타주
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* AttackMontage;
	
	bool bFocusPlayerAfterAttack = true; // 공격 후 플레이어 주시 여부
	UFUNCTION(BlueprintCallable)
	void StartFocusPlayerAfterAttack(); // 공격 후 플레이어 주시 시작 함수
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
