#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FireBall.generated.h"


UCLASS()
class ENEMY_API UBTTask_FireBall : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_FireBall();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
