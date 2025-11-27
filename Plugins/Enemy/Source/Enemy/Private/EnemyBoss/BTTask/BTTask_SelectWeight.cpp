#include "EnemyBoss/BTTask/BTTask_SelectWeight.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"

UBTTask_SelectWeight::UBTTask_SelectWeight()
{
	NodeName = "Select_Weight";
}

EBTNodeResult::Type UBTTask_SelectWeight::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// AI 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	// 컨트롤러가 제어하는 폰을 ABossSkeletonMage로 캐스팅
	ABossSkeletonMage* BossPawn = Cast<ABossSkeletonMage>(AIController->GetPawn());
	if (!BossPawn) return EBTNodeResult::Failed;
	
	// 블랙보드 컴포넌트 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;
	
	// 블랙보드에서 플레이어 위치 가져오기
	FVector PlayerPos = BlackboardComp->GetValueAsVector(PlayerLocation.SelectedKeyName);
	// 보스 위치 가져오기
	FVector BossPos = BossPawn->GetActorLocation();

	// 보스와 플레이어 사이의 거리 계산
	float Distance = FVector::Dist(PlayerPos, BossPos);

	//UE_LOG(LogTemp, Warning, TEXT("Distance: %f"), Distance );
	
	if (Distance <= 400.f) // 바로 앞
	{
		BlackboardComp->SetValueAsFloat(WeightKeys[4].SelectedKeyName, 1.f); // PushTarget
		BlackboardComp->SetValueAsFloat(WeightKeys[0].SelectedKeyName, 1.f); // Teleport
		BlackboardComp->SetValueAsFloat(WeightKeys[1].SelectedKeyName, 0.f); // FireBall
		BlackboardComp->SetValueAsFloat(WeightKeys[2].SelectedKeyName, 0.f); // Summon
		BlackboardComp->SetValueAsFloat(WeightKeys[3].SelectedKeyName, 0.f); // GroundAttack
		//UE_LOG( LogTemp, Warning, TEXT("a") );
	}
	else if (Distance > 400.f && Distance <= 1500.f) // 중거리
	{
		BlackboardComp->SetValueAsFloat(WeightKeys[4].SelectedKeyName, 0.f); // PushTarget
		BlackboardComp->SetValueAsFloat(WeightKeys[0].SelectedKeyName, 3.f); // Teleport
		BlackboardComp->SetValueAsFloat(WeightKeys[1].SelectedKeyName, 5.f); // FireBall
		BlackboardComp->SetValueAsFloat(WeightKeys[2].SelectedKeyName, 0.f); // Summon
		BlackboardComp->SetValueAsFloat(WeightKeys[3].SelectedKeyName, 2.f); // GroundAttack
		//UE_LOG( LogTemp, Warning, TEXT("b") );
	}
	else // 원거리
	{
		BlackboardComp->SetValueAsFloat(WeightKeys[4].SelectedKeyName, 0.f); // PushTarget
		BlackboardComp->SetValueAsFloat(WeightKeys[0].SelectedKeyName, 0.f); // Teleport
		BlackboardComp->SetValueAsFloat(WeightKeys[1].SelectedKeyName, 2.f); // FireBall
		BlackboardComp->SetValueAsFloat(WeightKeys[2].SelectedKeyName, 4.f); // Summon
		BlackboardComp->SetValueAsFloat(WeightKeys[3].SelectedKeyName, 5.f); // GroundAttack
		//UE_LOG( LogTemp, Warning, TEXT("c") );
	}

	return EBTNodeResult::Succeeded;
}
