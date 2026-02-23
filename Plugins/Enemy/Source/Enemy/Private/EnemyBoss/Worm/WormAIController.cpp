#include "EnemyBoss/Worm/WormAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/Worm/BossWorm.h"

void AWormAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ControlledEnemy = Cast<ABossWorm>(GetPawn());
}

void AWormAIController::SetBlackboardKey()
{
	Super::SetBlackboardKey();

	if ( BlackboardComp == nullptr || ControlledEnemy == nullptr ) return;

	BlackboardComp->SetValueAsBool("CanAttack", true);
	BlackboardComp->SetValueAsFloat("AttackDelay", ControlledEnemy->AttackDelay);

	BlackboardComp->SetValueAsObject(TEXT("TargetCharacter"), TargetPawn);
	BlackboardComp->SetValueAsBool( TEXT("bIsBurrowing"), ControlledEnemy->bIsBurrowing);
	BlackboardComp->SetValueAsFloat(TEXT("W_NormalAttack"), ControlledEnemy->AttackWeight.NormalAttack);
	BlackboardComp->SetValueAsFloat(TEXT("W_Burrow"), ControlledEnemy->AttackWeight.Burrow);
	BlackboardComp->SetValueAsFloat(TEXT("W_Unburrow"), ControlledEnemy->AttackWeight.Unburrow);	
	BlackboardComp->SetValueAsFloat(TEXT("W_RangedAttack"), ControlledEnemy->AttackWeight.RangedAttack);
	BlackboardComp->SetValueAsFloat(TEXT("W_LungeAttack"), ControlledEnemy->AttackWeight.LungeAttack);

}