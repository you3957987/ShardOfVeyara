// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyCropSlotBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Inventory/UI/AGSDDragVisualWidget.h"

void UAlchemyCropSlotBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (Texture && IMG_ItemIcon)
	{
		IMG_ItemIcon->SetBrushFromTexture(Texture);
	}
	if (TXT_ItemAmount)
	{
		TXT_ItemAmount->SetText(FText::AsNumber(Amount));
	}
}

void UAlchemyCropSlotBase::InitSlot(bool bInCantDrag)
{
	bCantDrag = bInCantDrag;
	
	if (bCantDrag)
	{
		// 드래그 불가 시 비주얼 처리가 필요하다면 여기서 수행합니다.
		// 예: 투명도나 회색조 머티리얼 적용 등
	}
}

void UAlchemyCropSlotBase::UpdateSlot(UTexture2D* InTexture, int32 InAmount)
{
	Texture = InTexture;
	Amount = InAmount;

	if (IMG_ItemIcon && Texture)
	{
		IMG_ItemIcon->SetBrushFromTexture(Texture);
	}

	if (TXT_ItemAmount)
	{
		TXT_ItemAmount->SetText(FText::AsNumber(Amount));
	}
}

FReply UAlchemyCropSlotBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !bCantDrag)
	{
		// 왼쪽 클릭이고 드래그가 가능한 상태라면 드래그 탐지 요청
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Reply;
}

void UAlchemyCropSlotBase::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (bCantDrag) return;

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>(this);
	if (!DragOp) return;

	// Payload로 자기 자신을 설정 (블루프린트 로직과 호환)
	DragOp->Payload = this;
	DragOp->Pivot = EDragPivot::MouseDown;

	// 드래그 비주얼 생성
	if (DragVisualClass)
	{
		UAGSDDragVisualWidget* DragVisual = CreateWidget<UAGSDDragVisualWidget>(GetWorld(), DragVisualClass);
		if (DragVisual)
		{
			// 데이터 세팅
			FStruct_ItemData TempItemData;
			TempItemData.ItemID = ItemID;
			TempItemData.CurrentQuantity = Amount;
			TempItemData.ItemIcon = Texture;
			
			DragVisual->SetDragItemData(TempItemData);
			DragOp->DefaultDragVisual = DragVisual;
		}
	}

	OutOperation = DragOp;
}
