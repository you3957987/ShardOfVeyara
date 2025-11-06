#include "EnemyBoss/SkeletonMage/BTTask_Teleport.h"
#include "AIController.h"
#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"
#include "NavigationSystem.h"

UBTTask_Teleport::UBTTask_Teleport()
{
    NodeName = "Teleport";
}

EBTNodeResult::Type UBTTask_Teleport::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // AI 컨트롤러 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    // 컨트롤러가 제어하는 폰을 ABossSkeletonMage로 캐스팅
    ABossSkeletonMage* BossPawn = Cast<ABossSkeletonMage>(AIController->GetPawn());
    if (!BossPawn)
    {
        return EBTNodeResult::Failed;
    }

    // 보스 자신의 현재 위치 가져오기
    const FVector BossLocation = BossPawn->GetActorLocation();

    // 내비게이션 시스템 가져오기
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem)
    {
        return EBTNodeResult::Failed;
    }

    FNavLocation RandomTeleportLocation;
    bool bFoundLocation = false;
    const int32 MaxAttempts = 10; // 최대 시도 횟수

    for (int32 i = 0; i < MaxAttempts; ++i)
    {
        // 보스 주변 최대 반경 내에서 이동 가능한 랜덤 위치 찾기
        if (NavSystem->GetRandomPointInNavigableRadius(BossLocation, BossPawn->MaxTeleportDist, RandomTeleportLocation))
        {
            // 현재 위치와 찾은 위치 사이의 거리가 최소 반경보다 큰지 확인
            if (FVector::Dist(BossLocation, RandomTeleportLocation.Location) > BossPawn->MinTeleportDist)
            {
                bFoundLocation = true;
                break; // 적절한 위치를 찾았으므로 반복 종료
            }
        }
    }
    if (bFoundLocation)
    {
        // 찾은 위치로 텔레포트 함수 호출
        BossPawn->PlayTeleportMontage(RandomTeleportLocation.Location);
        return EBTNodeResult::Succeeded;
    }

    // 적절한 위치를 찾지 못하면 실패 처리
    return EBTNodeResult::Failed;
}
