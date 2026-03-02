#pragma once

#include "CoreMinimal.h"
#include "HoldingState.h"
#include "Struct_ItemData.generated.h"

USTRUCT(BlueprintType)
struct FStruct_ItemData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// ItemID: 스트링 -> FString
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemID;

	// Item Category: EHolding State (Enum) -> 사용 중인 Enum 타입명으로 교체 필요
	// 일단 예시로 uint8 또는 해당 Enum 이름을 사용합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EHoldingState ItemCategory; 

	// Item Name: 텍스트 -> FText
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemName;

	// Item Description: 텍스트 -> FText
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemDescription;

	// Max Quantity: 인티저 -> int32
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxQuantity;

	// Current Quantity: 인티저 -> int32
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 CurrentQuantity;

	// Item Icon: 텍스처 2D -> UTexture2D*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<UTexture2D> ItemIcon;

	// Item BP Class: 액터 (클래스 레퍼런스) -> TSubclassOf<AActor>
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSubclassOf<AActor> ItemBPClass;

	// Is Equipment Equipped?: 부울 -> bool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool bIsEquipmentEquipped;

	// 기본값 설정을 위한 생성자
	FStruct_ItemData()
	{
		ItemID = TEXT("");
		ItemCategory = EHoldingState::EHS_None;
		ItemName = FText::GetEmpty();
		ItemDescription = FText::GetEmpty();
		MaxQuantity = 0;
		CurrentQuantity = 0;
		ItemIcon = nullptr;
		ItemBPClass = nullptr;
		bIsEquipmentEquipped = false;
	}
};
