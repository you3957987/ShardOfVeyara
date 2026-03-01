#pragma once

#include "CoreMinimal.h"
#include "Struct_ItemData.h"
#include "Struct_InventorySlotData.generated.h"

USTRUCT(BlueprintType)
struct FStruct_InventorySlotData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Data")
	bool isEmpty = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Data")
	FStruct_ItemData ItemData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Data")
	int32 SlotIndex;
};
