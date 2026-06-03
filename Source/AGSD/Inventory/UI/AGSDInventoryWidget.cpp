// AGSDInventoryWidget.cpp - 가방 인벤토리 UI 위젯 구현

#include "AGSDInventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Inventory/AGSDInventoryComponent.h"
#include "AGSDInventorySlotWidget.h"

void UAGSDInventoryWidget::InitializeInventory(UAGSDInventoryComponent* InInventoryComponent)
{
	if (!InInventoryComponent) return;

	InventoryComponent = InInventoryComponent;

	// 델리게이트 바인딩 (중복 등록 방지를 위해 먼저 해제)
	InventoryComponent->OnInventorySlotUpdated.RemoveAll(this);
	InventoryComponent->OnInventorySlotUpdated.AddDynamic(this, &UAGSDInventoryWidget::OnInventorySlotUpdated);

	InventoryComponent->OnInventoryFullyUpdated.RemoveAll(this);
	InventoryComponent->OnInventoryFullyUpdated.AddDynamic(this, &UAGSDInventoryWidget::OnInventoryFullyUpdated);

	// 그리드 패널 초기화
	if (UGP_InventorySlots)
	{
		UGP_InventorySlots->ClearChildren();
	}
	InventorySlotWidgets.Empty();

	// 가방 영역(10번 슬롯 이후) 생성 및 그리드 배치
	if (SlotWidgetClass)
	{
		const int32 MaxSlots = InventoryComponent->GetMaxSlots();
		const int32 SlotStart = StartSlotIndex;

		for (int32 i = SlotStart; i < MaxSlots; ++i)
		{
			UAGSDInventorySlotWidget* NewSlot = CreateWidget<UAGSDInventorySlotWidget>(this, SlotWidgetClass);
			if (NewSlot)
			{
				NewSlot->SlotIndex = i;
				NewSlot->OwningInventoryComponent = InventoryComponent;

				// 백엔드 데이터 동기화
				FStruct_InventorySlotData SlotData = InventoryComponent->GetSlotData(i);
				NewSlot->SetInventoryItem(SlotData.ItemData, SlotData.IsEmpty);

				// 행/열 좌표 계산 (기본 SlotsPerRow개씩 한 행에 배치)
				const int32 RelIndex = i - SlotStart;
				const int32 Row = RelIndex / SlotsPerRow;
				const int32 Col = RelIndex % SlotsPerRow;

				UGP_InventorySlots->AddChildToUniformGrid(NewSlot, Row, Col);
				InventorySlotWidgets.Add(NewSlot);
			}
		}
	}
}

void UAGSDInventoryWidget::RefreshInventory()
{
	if (!InventoryComponent) return;

	const int32 SlotStart = StartSlotIndex;

	for (int32 i = 0; i < InventorySlotWidgets.Num(); ++i)
	{
		const int32 SlotIndex = SlotStart + i;
		if (InventoryComponent->IsValidSlotIndex(SlotIndex))
		{
			FStruct_InventorySlotData SlotData = InventoryComponent->GetSlotData(SlotIndex);
			InventorySlotWidgets[i]->SetInventoryItem(SlotData.ItemData, SlotData.IsEmpty);
		}
	}
}

void UAGSDInventoryWidget::OnInventorySlotUpdated(int32 SlotIndex)
{
	if (!InventoryComponent) return;

	const int32 SlotStart = StartSlotIndex;

	// 업데이트된 슬롯이 가방 영역에 해당하는 경우에만 처리
	if (SlotIndex >= SlotStart && SlotIndex < InventoryComponent->GetMaxSlots())
	{
		const int32 WidgetIndex = SlotIndex - SlotStart;
		if (InventorySlotWidgets.IsValidIndex(WidgetIndex))
		{
			FStruct_InventorySlotData SlotData = InventoryComponent->GetSlotData(SlotIndex);
			InventorySlotWidgets[WidgetIndex]->SetInventoryItem(SlotData.ItemData, SlotData.IsEmpty);
		}
	}
}

void UAGSDInventoryWidget::OnInventoryFullyUpdated()
{
	RefreshInventory();
}

void UAGSDInventoryWidget::NativeDestruct()
{
	// 델리게이트 해제 (댕글링 콜백 방지)
	if (InventoryComponent)
	{
		InventoryComponent->OnInventorySlotUpdated.RemoveDynamic(this, &UAGSDInventoryWidget::OnInventorySlotUpdated);
		InventoryComponent->OnInventoryFullyUpdated.RemoveDynamic(this, &UAGSDInventoryWidget::OnInventoryFullyUpdated);
	}

	Super::NativeDestruct();
}
