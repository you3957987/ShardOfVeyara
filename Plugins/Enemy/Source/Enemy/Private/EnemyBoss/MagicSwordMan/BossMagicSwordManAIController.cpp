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
	BlackboardComp->SetValueAsFloat("W_DashAttack", ControlledEnemy->AttackWeight.DashAttack);
	BlackboardComp->SetValueAsBool("bAirAttack", false);
	BlackboardComp->SetValueAsFloat("W_CloseJumpUpAttack", ControlledEnemy->AttackWeight.CloseJumpUpAttack);
	BlackboardComp->SetValueAsFloat("W_DashJumpUpAttack", ControlledEnemy->AttackWeight.DashJumpUpAttack);
	BlackboardComp->SetValueAsFloat("W_JumpAttack", ControlledEnemy->AttackWeight.JumpAttack);	
	BlackboardComp->SetValueAsFloat("W_Guard", ControlledEnemy->AttackWeight.Guard);
	BlackboardComp->SetValueAsFloat("W_PowerAttack", ControlledEnemy->AttackWeight.PowerAttack);
	
}
