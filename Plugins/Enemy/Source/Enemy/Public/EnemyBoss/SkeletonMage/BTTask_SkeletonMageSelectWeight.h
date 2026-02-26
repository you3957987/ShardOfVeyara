#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_SkeletonMageSelectWeight.generated.h"

USTRUCT(BlueprintType)
struct FSkeletonMageWeightConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	FBlackboardKeySelector Key;

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float Value = 0.f;
};

UCLASS()
class ENEMY_API UBTTask_SkeletonMageSelectWeight : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SkeletonMageSelectWeight();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 근거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	float CloseRangeDistance = 600.f;
	// 중거리	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	float MidRangeDistance = 1800.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="자체설정")
	FBlackboardKeySelector PlayerLocation;

	// 근거리일 때 적용할 키-값 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정", meta=(TitleProperty="Key"))
	TArray<FSkeletonMageWeightConfig> CloseRangeWeights;	
	
	// 중거리일 때 적용할 키-값 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정", meta=(TitleProperty="Key"))
	TArray<FSkeletonMageWeightConfig> MidRangeWeights;
	
	// 원거리일 때 적용할 키-값 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정", meta=(TitleProperty="Key"))
	TArray<FSkeletonMageWeightConfig> FarRangeWeights;
};
