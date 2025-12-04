#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SkeletonMageSelectWeight.generated.h"

UCLASS()
class ENEMY_API UBTTask_SkeletonMageSelectWeight : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SkeletonMageSelectWeight();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	FBlackboardKeySelector PlayerLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TArray<FBlackboardKeySelector> WeightKeys;
};
