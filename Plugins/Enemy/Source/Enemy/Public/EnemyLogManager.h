#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnemyLogManager.generated.h"

// 로그의 종류를 정의하는 열거형 (파일 이름으로 매핑됨)
UENUM(BlueprintType)
enum class EEnemyLogType : uint8
{
	// 적 로그
	Melee,
	Ranged,
	Exploder,
	Transpar,
	Mimic,
	Slime,
	Mage,
	Guard,
	Passive,
	Burrow,
	Revive,
	// 대미지 얼마 주었는지
	// 대미지 얼마 받았는지 + 사망 여부
	// 전투 시작 시간
	// 전투 종료 시간 및 전투 시간
	// 부활몹 부활 했는지
	// 방패병 가드 헸는지
	Spawner,
	// 몬스터 스폰 했는지
	
	
	// 보스 로그
	SkeletonMage,
	BlackKnight,
	Worm,
	MagicSwordMan
};

UCLASS()
class ENEMY_API UEnemyLogManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static FString GetLogFileName(EEnemyLogType InEnemyLogType);
	
public:

	static void EnemyLog(EEnemyLogType InEnemyLogType, FString Content);
	
};