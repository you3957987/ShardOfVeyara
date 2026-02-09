#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_SetDistanceToCharacter.generated.h"


UCLASS()
class ENEMY_API UBTService_SetDistanceToCharacter : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_SetDistanceToCharacter();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 에디터에서 선택: 타깃 플레이어가 저장된 키 (Input)
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetKey;

	// 에디터에서 선택: 계산된 거리를 저장할 키 (Output)
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector DistanceKey;
};
