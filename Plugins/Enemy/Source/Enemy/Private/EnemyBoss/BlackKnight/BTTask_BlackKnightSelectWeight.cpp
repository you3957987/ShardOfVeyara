#include "EnemyBoss/BlackKnight/BTTask_BlackKnightSelectWeight.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/BlackKnight/BossBlackKnight.h"

UBTTask_BlackKnightSelectWeight::UBTTask_BlackKnightSelectWeight()
{
	NodeName = "SkeletonMage_Select_Weight";
}

EBTNodeResult::Type UBTTask_BlackKnightSelectWeight::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// AI 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	// 컨트롤러가 제어하는 폰을 ABossSkeletonMage로 캐스팅
	ABossBlackKnight* BossPawn = Cast<ABossBlackKnight>(AIController->GetPawn());
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
	
	if (Distance <= CloseRangeDistance) // 근거리
	{
		BlackboardComp->SetValueAsFloat(WeightKeys[0].SelectedKeyName, 3.f); // W_NormalAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[1].SelectedKeyName, 2.f); // W_ChargeAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[2].SelectedKeyName, 2.f); // W_Guard
		BlackboardComp->SetValueAsFloat(WeightKeys[3].SelectedKeyName, 0.f); // W_RushAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[4].SelectedKeyName, 0.f); // W_ZapAttack
		//UE_LOG( LogTemp, Warning, TEXT("a") );
	}
	else if (Distance > CloseRangeDistance && Distance <= MidRangeDistance) // 중거리
	{
		BlackboardComp->SetValueAsFloat(WeightKeys[0].SelectedKeyName, 0.f); // W_NormalAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[1].SelectedKeyName, 5.f); // W_ChargeAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[2].SelectedKeyName, 1.f); // W_Guard
		BlackboardComp->SetValueAsFloat(WeightKeys[3].SelectedKeyName, 2.f); // W_RushAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[4].SelectedKeyName, 2.f); // W_ZapAttack
		//UE_LOG( LogTemp, Warning, TEXT("b") );
	}
	else // 원거리
	{
		BlackboardComp->SetValueAsFloat(WeightKeys[0].SelectedKeyName, 0.f); // W_NormalAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[1].SelectedKeyName, 0.f); // W_ChargeAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[2].SelectedKeyName, 0.f); // W_Guard
		BlackboardComp->SetValueAsFloat(WeightKeys[3].SelectedKeyName, 5.f); // W_RushAttack
		BlackboardComp->SetValueAsFloat(WeightKeys[4].SelectedKeyName, 2.f); // W_ZapAttack
		//UE_LOG( LogTemp, Warning, TEXT("c") );
	}

	return EBTNodeResult::Succeeded;
}