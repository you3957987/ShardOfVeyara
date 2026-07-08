#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SpearComboData.generated.h"

UENUM(BlueprintType)
enum class ESpearAttackDirection : uint8
{
	Neutral,
	Forward,
	Backward,
	Sprint,
	Parry
};

USTRUCT(BlueprintType)
struct FSpearStageData // 각 단계별 개별 애니메이션 데이터
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackMontage; // 공격 동작 (Stage)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FSpearComboData : public FTableRowBase
{
	GENERATED_BODY()

	// 콤보 하나(1~10번) 내의 모든 스테이지를 리스트로 관리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSpearStageData> Stages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpearAttackDirection DirectionRequirement = ESpearAttackDirection::Neutral;
};