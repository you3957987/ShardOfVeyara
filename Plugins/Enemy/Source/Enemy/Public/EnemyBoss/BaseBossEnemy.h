#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyLogManager.h"
#include "BaseBossEnemy.generated.h"

UCLASS()
class ENEMY_API ABaseBossEnemy : public ACharacter
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="자체설정")
	bool bCheckDeadLogic = false;
	void TestDeadLogic(); // 테스트용 죽음 로직 함수

	// 디버그 모드
	UPROPERTY(EditAnywhere, Category="자체설정")
	bool bDebugMode = false;
	
	// 플레이어 인식 후 AI 행동 시작 범위 스피어
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* PlayerDetectRangeSphere;
	// 플레이어 인식 후 AI 행동 중지 범위
	UPROPERTY(EditAnywhere, Category="자체설정")
	float PlayerDetectRange = 1200.f;
	// 플레이어가 감지 범위에 들어왔을 때 호출될 함수
	UFUNCTION()
	void OnPlayerDetectOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// 체력 바 위젯 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<class UWidgetComponent> HealthBarWidget;
	// Tick에서 체력 바 위젯을 내 캐릭터 쪽으로 돌아보게 하는 함수
	void UpdateHealthBarWidget(float DeltaTime);
	
	// 최대 체력
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MaxHealth = 100.f;
	// 현재 체력
	UPROPERTY(EditAnywhere, Category="자체설정")
	float Health = 100.f;
	// 이동 속도 = 여기서 값 바꾸면 자동으로 캐릭터 무브먼트 컴포넌트의 MaxWalkSpeed도 바뀜
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MoveSpeed = 300.f;
	
	// 타겟으로 삼을 캐릭터를 저장할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<ACharacter> TargetCharacter;
	// 블랙보드 컴포넌트
	class UBlackboardComponent* BlackboardComp;
	// 블랙보드 컴포넌트가 설정되었는지 여부
	bool bSetBlackboard = false; 
	// 타겟 캐릭터가 설정되었는지 여부
	bool bTargetInitalize = false;
	// 틱에서 하는 초기화
	void PollInit(float DeltaTime); 

	// 죽음 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UAnimMontage* DeathMontage;
	// 죽을 떄 생성할 이펙트 == 케스케이드
	UPROPERTY(EditDefaultsOnly, Category="자체설정")
	class UParticleSystem* DeathEffectCascade;
	// 죽음 이펙트 생성 앞 뒤 위치 조정용 거리
	UPROPERTY(EditDefaultsOnly, Category="자체설정")
	float DeathEffectForwardOffset = 0.0f;
	// 죽음 이펙트 크기 배율
	UPROPERTY(EditDefaultsOnly, Category="자체설정")
	float DeathEffectScale = 1.0f;
	// 죽음 후 일정 시간 뒤에 이펙트 생성 및 액터 제거를 위한 타이머 핸들
	FTimerHandle DeathTimerHandle;
	void SpawnDeadEffectAndDestroy();
	// 죽고 나서 떨어질 아이템 배열
	UPROPERTY(EditAnywhere, Category="자체설정")
	TArray<TSubclassOf<AActor>> DropItems;
	// 아이템 드롭 함수
	void DropItemsAfterDead();

public:
	ABaseBossEnemy();
	virtual void Tick(float DeltaTime) override;
	
	// 데미지 처리 함수 재정의
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;
	
	UPROPERTY(VisibleAnywhere)
	class UMotionWarpingComponent* MotionWarpingComponent;
	
	// 모션 워핑할 타깃 캐릭터 좌표 찍기
	UFUNCTION(BlueprintCallable)
	void UpdateMotionWarpTarget();
	// 모션 워핑 타깃 - 타깃 캐릭터의 앞쪽
	UFUNCTION(BlueprintCallable)
	void UpdateMotionWarpTargetToFront();
	
	// 공격 지연 시간. 블랙보드 태스크에서 재지정
	float AttackDelay = 0.0f;
	
	// 죽음 처리 함수
	void Die();
	// 죽음 몽타주 끝난 후 호출되는 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void AfterDieMontageEnd();
	
	// 공격 후 플레이어 주시 여부
	bool bFocusPlayerAfterAttack = true;
	// 공격 후 플레이어 주시 시작 함수
	UFUNCTION(BlueprintCallable)
	void StartFocusPlayerAfterAttack(); 
	
	// 로그 관리 
	float BattleStartTime = 0.f;
	bool bIsInBattle = false;
	void StartBattleLog();
	void EndBattleLog();
	void AttackPatternLog(FString PatternName) const;
	FString SelectedRangeName = TEXT("플레이어 첫 조우");
	EEnemyLogType GetLogTypeFromEnemyType() const;
	UFUNCTION()
	void PlayerDeadLog();
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};






