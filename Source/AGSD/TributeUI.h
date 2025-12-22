// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TributeItem.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "TributeUI.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTributeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	TMap<FString, int32> TributeItems;

	FTributeData() : TributeItems({}) {}
};

UCLASS()
class AGSD_API UTributeUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UUniformGridPanel* TributeItemGrid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<class UTributeItem*> TributeItems;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	TSubclassOf<UTributeItem> TributeItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	UDataTable* ItemDataTable;

public:
	void SetNextTributeItem(const TMap<FString, int32>& ItemMap);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tribute")
	void SetTributeItemData(const FString& ItemID, int32 Amount, UTributeItem* TargetWidget);
};