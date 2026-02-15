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
}
