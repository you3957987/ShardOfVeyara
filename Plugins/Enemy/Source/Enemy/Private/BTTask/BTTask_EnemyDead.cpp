#include "BTTask/BTTask_EnemyDead.h"
#include "AIController.h"
#include "BaseEnemy.h"

UBTTask_EnemyDead::UBTTask_EnemyDead()
{
	NodeName = "EnemyDead";

	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_EnemyDead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	OwnerComp.StopTree(); // 비헤이비어 트리 중지
	
	// 태스크를 'InProgress' 상태로 만들어 행동 트리가 여기에 머물도록 합니다.
	return EBTNodeResult::InProgress;
}
