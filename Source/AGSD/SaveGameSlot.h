// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveGameSlot.generated.h"

/**
 * 
 */
UCLASS()
class AGSD_API USaveGameSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGameSlot")
	int32 SlotIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGameSlot")
	FString SlotName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGameSlot")
	bool bIsEmpty = true;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* SlotNum;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* EmptyText;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UHorizontalBox* SaveDataPanel;

protected:
	virtual void NativeConstruct() override;
};
