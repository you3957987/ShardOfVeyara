#include "EnemyBoss/BlackKnight/BTTask_BlackKnightSelectWeight.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"		

UBTTask_BlackKnightSelectWeight::UBTTask_BlackKnightSelectWeight()
{
	NodeName = "BlackKnight_Select_Weight";
}

EBTNodeResult::Type UBTTask_BlackKnightSelectWeight::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	// 거리 계산
	FVector PlayerPos = BlackboardComp->GetValueAsVector(PlayerLocation.SelectedKeyName);
	FVector BossPos = ControlledPawn->GetActorLocation();
	float Distance = FVector::Dist(PlayerPos, BossPos);

	// 설정 적용을 위한 람다 함수 (중복 제거용)
	auto ApplyWeights = [&](const TArray<FBlackKnightWeightConfig>& Settings)
	{
		for (const FBlackKnightWeightConfig& Config : Settings)
		{
			if (Config.Key.SelectedKeyName != NAME_None)
			{
				BlackboardComp->SetValueAsFloat(Config.Key.SelectedKeyName, Config.Value);
			}
		}
	};

	// 거리별 분기 처리
	if (Distance <= CloseRangeDistance) // 근거리
	{
		ApplyWeights(CloseRangeWeights);
		// 로그
		UE_LOG(LogTemp, Warning, TEXT("Distance: %f - Close Range Weights Applied"), Distance);
	}
	else if (Distance > CloseRangeDistance && Distance <= MidRangeDistance) // 중거리
	{
		ApplyWeights(MidRangeWeights);
		// 로그
		UE_LOG(LogTemp, Warning, TEXT("Distance: %f - Mid Range Weights Applied"), Distance);
	}
	else // 원거리
	{
		ApplyWeights(FarRangeWeights);
		// 로그
		UE_LOG(LogTemp, Warning, TEXT("Distance: %f - Far Range Weights Applied"), Distance);
	}

	return EBTNodeResult::Succeeded;
}