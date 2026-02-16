#include "EnemyBoss/BlackKnight/BossBlackKnightAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/BlackKnight/BossBlackKnight.h"

void ABossBlackKnightAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Super::OnPossess(InPawn);
	ControlledEnemy = Cast<ABossBlackKnight>(GetPawn());
}

void ABossBlackKnightAIController::SetBlackboardKey()
{
	Super::SetBlackboardKey();

	if ( BlackboardComp == nullptr || ControlledEnemy == nullptr ) return;

	BlackboardComp->SetValueAsBool("CanAttack", true);
	BlackboardComp->SetValueAsFloat("AttackDelay", ControlledEnemy->AttackDelay);

	BlackboardComp->SetValueAsObject(TEXT("TargetCharacter"), TargetPawn);
	BlackboardComp->SetValueAsFloat(TEXT("W_RushAttack"), ControlledEnemy->AttackWeight.RushAttack);
	BlackboardComp->SetValueAsFloat(TEXT("W_Guard"), ControlledEnemy->AttackWeight.Guard);
	BlackboardComp->SetValueAsFloat( TEXT("W_NormalAttack"), ControlledEnemy->AttackWeight.NormalAttack );
	BlackboardComp->SetValueAsFloat( TEXT("W_ChargeAttack"), ControlledEnemy->AttackWeight.ChargeAttack);
	BlackboardComp->SetValueAsFloat( TEXT("W_ZapAttack"), ControlledEnemy->AttackWeight.ZapAttack );
}
