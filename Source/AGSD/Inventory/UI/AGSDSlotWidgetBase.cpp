// AGSDSlotWidgetBase.cpp - 공통 슬롯 위젯 베이스 클래스 구현

#include "AGSDSlotWidgetBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/AGSDInventoryComponent.h"
#include "AGSDSlotDragDropOperation.h"
#include "AGSDDragVisualWidget.h"
#include "Input/Events.h"
#include "Struct_ItemData.h"

void UAGSDSlotWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisual();
}

void UAGSDSlotWidgetBase::SetItemData(FStruct_ItemData ItemData, bool bClear)
{
	if (bClear)
	{
		SlotItemData.IsEmpty = true;
		SlotItemData.ItemData = FStruct_ItemData();
		SlotItemData.SlotIndex = SlotIndex;
	}
	else
	{
		SlotItemData.IsEmpty = false;
		SlotItemData.ItemData = ItemData;
		SlotItemData.SlotIndex = SlotIndex;
	}
	UpdateVisual();
}

void UAGSDSlotWidgetBase::UpdateVisual()
{
	if (SlotItemData.IsEmpty || SlotItemData.ItemData.ItemID.IsEmpty())
	{
		if (IMG_ItemIcon)
		{
			IMG_ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (TXT_ItemAmount)
		{
			TXT_ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (IMG_ItemIcon)
		{
			IMG_ItemIcon->SetVisibility(ESlateVisibility::Visible);
			if (SlotItemData.ItemData.ItemIcon)
			{
				IMG_ItemIcon->SetBrushFromTexture(SlotItemData.ItemData.ItemIcon);
			}
		}

		if (TXT_ItemAmount)
		{
			// MaxQuantity가 1보다 크고, 현재 수량이 1개보다 많을 때만 수량 텍스트를 노출합니다.
			if (SlotItemData.ItemData.MaxQuantity > 1 && SlotItemData.ItemData.CurrentQuantity >= 1)
			{
				TXT_ItemAmount->SetVisibility(ESlateVisibility::Visible);
				TXT_ItemAmount->SetText(FText::AsNumber(SlotItemData.ItemData.CurrentQuantity));
			}
			else
			{
				TXT_ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

FReply UAGSDSlotWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !SlotItemData.IsEmpty)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			const float CurrentTime = World->GetTimeSeconds();
			const FVector2D CurrentPosition = InMouseEvent.GetScreenSpacePosition();

			// 0.3초 이내에 10픽셀 이내의 동일 영역 클릭 시 더블 클릭으로 직접 판정하여 드래그 상태와의 충돌을 차단합니다.
			if (CurrentTime - LastClickTime < 0.3f && FVector2D::Distance(CurrentPosition, LastClickPosition) < 10.f)
			{
				// 더블클릭 이벤트 델리게이트 브로드캐스트 호출 (상자 및 인벤토리 빠른 이동 연동용)
				OnSlotDoubleClicked.Broadcast(this);
				
				LastClickTime = 0.f; // 더블클릭 처리 후 초기화
				return FReply::Handled();
			}

			LastClickTime = CurrentTime;
			LastClickPosition = CurrentPosition;
		}

		// 드래그 디텍팅 트리거 설정 (더블클릭이 아닐 시 드래그 판단 개시)
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

void UAGSDSlotWidgetBase::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UAGSDSlotDragDropOperation* DragOp = NewObject<UAGSDSlotDragDropOperation>(this);
	if (DragOp)
	{
		DragOp->SourceSlotIndex = SlotIndex;
		DragOp->SourceSlotData = SlotItemData;
		DragOp->SourceWidget = this;
		
		// 인벤토리 컴포넌트 핫바 영역 상수 범위(0~9)로 핫바 출발 여부 판단
		DragOp->bFromHotbar = (SlotIndex >= 0 && SlotIndex <= 9);

		// 드래그 비주얼 생성 및 아이템 데이터 주입
		if (DragVisualClass)
		{
			// UAGSDDragVisualWidget으로 직접 생성하여 타입 안전하게 데이터를 전달합니다.
			if (UAGSDDragVisualWidget* DragVisual = CreateWidget<UAGSDDragVisualWidget>(this, DragVisualClass))
			{
				DragVisual->SetDragItemData(SlotItemData.ItemData);
				DragOp->DefaultDragVisual = DragVisual;
				// CenterCenter: 위젯 중심이 마우스 커서에 오도록 설정
				DragOp->Pivot = EDragPivot::CenterCenter;
			}
		}

		OutOperation = DragOp;
	}
}


void UAGSDSlotWidgetBase::NativeOnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation)
{
	Super::NativeOnDragEnter(MyGeometry, DragDropEvent, Operation);
}

void UAGSDSlotWidgetBase::NativeOnDragLeave(const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation)
{
	Super::NativeOnDragLeave(DragDropEvent, Operation);
}

bool UAGSDSlotWidgetBase::NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation)
{
	Super::NativeOnDrop(MyGeometry, DragDropEvent, Operation);

	if (UAGSDSlotDragDropOperation* SlotOp = Cast<UAGSDSlotDragDropOperation>(Operation))
	{
		if (UAGSDInventoryComponent* InvComp = GetInventoryComponent())
		{
			// 백엔드 데이터 스왑 수행
			InvComp->SwapSlots(SlotOp->SourceSlotIndex, SlotIndex);
			return true;
		}
	}

	return false;
}

UAGSDInventoryComponent* UAGSDSlotWidgetBase::GetInventoryComponent() const
{
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return PlayerPawn->FindComponentByClass<UAGSDInventoryComponent>();
	}
	return nullptr;
}
