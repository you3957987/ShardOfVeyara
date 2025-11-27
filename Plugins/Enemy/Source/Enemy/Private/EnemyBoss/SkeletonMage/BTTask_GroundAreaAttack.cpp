#include "EnemyBoss/SkeletonMage/BTTask_GroundAreaAttack.h"
#include "AIController.h"
#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"

UBTTask_GroundAreaAttack::UBTTask_GroundAreaAttack()
{
	NodeName = "GroundAreaAttack";
}

EBTNodeResult::Type UBTTask_GroundAreaAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// AI 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	// 컨트롤러가 제어하는 폰을 ABossSkeletonMage로 캐스팅
	ABossSkeletonMage* BossPawn = Cast<ABossSkeletonMage>(AIController->GetPawn());
	if (!BossPawn)
	{
		return EBTNodeResult::Failed;
	}

	if ( BossPawn )
	{
		BossPawn->PlayGroundAreaAttackMontage();
	}

	return EBTNodeResult::Succeeded;
}
