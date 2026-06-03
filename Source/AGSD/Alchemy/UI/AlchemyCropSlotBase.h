// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlchemyCropSlotBase.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UAlchemyInventoryUI;

/**
 * UAlchemyCropSlotBase
 * WBP_AlchemyCropSlotBase 위젯의 C++ 부모 클래스입니다.
 */
UCLASS()
class AGSD_API UAlchemyCropSlotBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── UI 바인딩 ──
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UImage> IMG_ItemIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UTextBlock> TXT_ItemAmount;

	// ── 슬롯 상태 및 데이터 ──
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemy Slot")
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemy Slot")
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemy Slot")
	TObjectPtr<UTexture2D> Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemy Slot")
	bool bCantDrag = false;

	UPROPERTY(BlueprintReadWrite, Category = "Alchemy Slot")
	TObjectPtr<UAlchemyInventoryUI> AlchemyInventoryUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemy Slot")
	bool bIsInsertedSlot = false;

	// ── 위젯 인터페이스 함수 ──
	UFUNCTION(BlueprintCallable, Category = "Alchemy Slot")
	void InitSlot(bool bInCantDrag, bool bInIsInsertedSlot = false);

	UFUNCTION(BlueprintCallable, Category = "Alchemy Slot")
	void UpdateSlot(UTexture2D* InTexture, int32 InAmount);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Alchemy Drag")
	TSubclassOf<class UAGSDDragVisualWidget> DragVisualClass;
};
