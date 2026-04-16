#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Struct_ItemData.h"
#include "SpearComboData.generated.h"

USTRUCT(BlueprintType)
struct FSpearComboData : public FTableRowBase
{
	GENERATED_BODY()

	// 1. 입력 조건
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredInput; // "Attack", "Forward", "Backward" 등

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresForward; // 전진 키가 필요한지 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsChargeAttack; // 차징 공격 여부

	// 2. 실행 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ComboMontage;

	// 3. 게임플레이 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NextComboSection; // 다음 연계될 섹션 이름
};