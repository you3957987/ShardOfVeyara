#include "EnemyBoss/SkeletonMage/BossSkeletonMageAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"


void ABossSkeletonMageAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<ABossSkeletonMage>(GetPawn());
}

void ABossSkeletonMageAIController::SetBlackboardKey()
{
	Super::SetBlackboardKey();

	if ( BlackboardComp == nullptr || ControlledEnemy == nullptr ) return;
	
	BlackboardComp->SetValueAsFloat("W_Teleport", ControlledEnemy->AttackWeight.Teleport);
	BlackboardComp->SetValueAsFloat("W_FireBall", ControlledEnemy->AttackWeight.FireBall);
	BlackboardComp->SetValueAsFloat("W_Summon", ControlledEnemy->AttackWeight.Summon);
	BlackboardComp->SetValueAsFloat("W_GroundAttack", ControlledEnemy->AttackWeight.GroundAttack);
	BlackboardComp->SetValueAsFloat("W_PushTarget", ControlledEnemy->AttackWeight.PushTarget);

	BlackboardComp->SetValueAsFloat("AttackDelay", ControlledEnemy->AttackDelay);
	BlackboardComp->SetValueAsBool("CanAttack", true);
}
