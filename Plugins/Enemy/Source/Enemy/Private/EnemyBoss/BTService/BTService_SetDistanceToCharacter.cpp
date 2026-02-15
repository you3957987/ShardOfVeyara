#include "EnemyBoss/BTService/BTService_SetDistanceToCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SetDistanceToCharacter::UBTService_SetDistanceToCharacter()
{
	
}

void UBTService_SetDistanceToCharacter::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 1. 내 AI Pawn 가져오기
	APawn* ControllingPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (nullptr == ControllingPawn) return;

	// 2. 블랙보드 컴포넌트 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (nullptr == BlackboardComp) return;

	// 3. 타깃 위치 가져오기
	
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (TargetActor)
	{
		float Dist = FVector::Dist(ControllingPawn->GetActorLocation(), TargetActor->GetActorLocation());
        
		// 4. 거리값 저장
		BlackboardComp->SetValueAsFloat(DistanceKey.SelectedKeyName, Dist);
	}
	
}
