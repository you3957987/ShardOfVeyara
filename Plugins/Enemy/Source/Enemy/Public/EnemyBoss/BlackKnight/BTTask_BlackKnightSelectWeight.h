#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_BlackKnightSelectWeight.generated.h"


UCLASS()
class ENEMY_API UBTTask_BlackKnightSelectWeight : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_BlackKnightSelectWeight();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 근거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	float CloseRangeDistance = 400.f;
	// 중거리	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	float MidRangeDistance = 1500.f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	FBlackboardKeySelector PlayerLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TArray<FBlackboardKeySelector> WeightKeys;
};
