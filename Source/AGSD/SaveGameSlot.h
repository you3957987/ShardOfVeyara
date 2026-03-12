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
	class UButton* SlotButton;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* SlotNum;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UHorizontalBox* SaveDataPanel;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* PlayMap;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* PlayDay;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* PlayTime;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* LastSaveTime;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* EmptyText;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickEmptySlotButton();

	UFUNCTION()
	void OnClickSavedSlotButton();
};
