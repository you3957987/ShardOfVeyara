#include "EnemyBoss/MagicSwordMan/BossMagicSwordManAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/MagicSwordMan/BossMagicSwordMan.h"

void ABossMagicSwordManAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<ABossMagicSwordMan>(GetPawn());
}

void ABossMagicSwordManAIController::SetBlackboardKey()
{
	Super::SetBlackboardKey();
	
	if ( BlackboardComp == nullptr || ControlledEnemy == nullptr ) return;

	BlackboardComp->SetValueAsBool("CanAttack", true);
	BlackboardComp->SetValueAsFloat("AttackDelay", ControlledEnemy->AttackDelay);
	BlackboardComp->SetValueAsObject(TEXT("TargetCharacter"), TargetPawn);
	
	BlackboardComp->SetValueAsFloat("W_CloseAttack", ControlledEnemy->AttackWeight.CloseAttack);
}
