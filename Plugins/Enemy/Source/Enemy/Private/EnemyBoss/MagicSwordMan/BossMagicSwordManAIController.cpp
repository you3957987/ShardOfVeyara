#include "EnemyBoss/MagicSwordMan/BossMagicSwordManAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/MagicSwordMan/BossMagicSwordMan.h"

// 깃 추가 확인용
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
	
	BlackboardComp->SetValueAsFloat("W_CloseAttack", ControlledEnemy->AttackStruct.CloseAttack);
	BlackboardComp->SetValueAsFloat("W_DashAttack", ControlledEnemy->AttackStruct.DashAttack);
	BlackboardComp->SetValueAsBool("bAirAttack", false);
	BlackboardComp->SetValueAsFloat("W_CloseJumpUpAttack", ControlledEnemy->AttackStruct.CloseJumpUpAttack);
	BlackboardComp->SetValueAsFloat("W_DashJumpUpAttack", ControlledEnemy->AttackStruct.DashJumpUpAttack);
	BlackboardComp->SetValueAsFloat("W_JumpAttack", ControlledEnemy->AttackStruct.JumpAttack);	
	BlackboardComp->SetValueAsFloat("W_Guard", ControlledEnemy->AttackStruct.Guard);
	BlackboardComp->SetValueAsFloat("W_PowerAttack", ControlledEnemy->AttackStruct.PowerAttack);
	BlackboardComp->SetValueAsFloat( "W_BladeWaveAttack", ControlledEnemy->AttackStruct.BladeWaveAttack);
	
}
