#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	// 체력 바 위젯 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "자체설정")
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
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* DeathMontage;
	// 죽을 떄 생성할 이펙트 == 케스케이드
	UPROPERTY(EditAnywhere, Category="자체설정")
	class UParticleSystem* DeathEffectCascade;
	// 죽음 이펙트 생성 앞 뒤 위치 조정용 거리
	UPROPERTY(EditAnywhere, Category="자체설정")
	float DeathEffectForwardOffset = 0.0f;
	// 죽음 이펙트 크기 배율
	UPROPERTY(EditAnywhere, Category="자체설정")
	float DeathEffectScale = 1.0f;
	// 죽음 후 일정 시간 뒤에 이펙트 생성 및 액터 제거를 위한 타이머 핸들
	FTimerHandle DeathTimerHandle;
	void SpawnDeadEffectAndDestroy();
	
public:
	ABaseBossEnemy();
	virtual void Tick(float DeltaTime) override;
	
	// 데미지 처리 함수 재정의
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;
	
	// 공격 지연 시간. 블랙보드 태스크에서 재지정
	float AttackDelay = 0.0f;
	
	// 죽음 처리 함수
	void Die();
	// 죽음 몽타주 끝난 후 호출되는 함수 - 애님 노티파이에서 호출
	UFUNCTION(BlueprintCallable)
	void AfterDieMontageEnd();
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};






