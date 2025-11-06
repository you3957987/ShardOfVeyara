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
	bool bSetBlackboard = false; // 블랙보드 컴포넌트가 설정되었는지 여부

	bool bTargetInitalize = false; // 타겟 캐릭터가 설정되었는지 여부

	void PollInit(float DeltaTime); // 틱에서 하는 초기화
public:
	ABaseBossEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 공격 딜레이
	UPROPERTY(EditAnywhere, Category="자체설정")
	float AttackDelay = 1.5f;

#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
