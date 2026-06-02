// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyInventoryUI.h"
#include "Components/UniformGridPanel.h"
#include "Engine/DataTable.h"
#include "Struct_ItemData.h"
#include "AlchemyCropSlotBase.h"

void UAlchemyInventoryUI::InitializeSlots(const TMap<FString, FStruct_MaterialAddress>& MaterialAddress, EHoldingState Category, bool bInsertedSlot)
{
	if (!UGP_InventorySlots) return;
	if (!CropSlotClass) return;

	UGP_InventorySlots->ClearChildren();
	Slots.Empty();

	int32 Column = 0;

	for (const TPair<FString, FStruct_MaterialAddress>& Pair : MaterialAddress)
	{
		const FString& ItemID = Pair.Key;
		const FStruct_MaterialAddress& MatAddress = Pair.Value;

		// 데이터 테이블에서 아이템 정보 조회
		FStruct_ItemData* ItemData = nullptr;
		if (ItemDataTable)
		{
			ItemData = ItemDataTable->FindRow<FStruct_ItemData>(FName(*ItemID), TEXT("InitializeSlots"));
		}

		if (ItemData)
		{
			// EHoldingState 카테고리 필터링 비교 (하이브리드 호환)
			if (Category == EHoldingState::EHS_None || 
				ItemData->EquipHoldingState == Category || 
				ItemData->ItemCategory == Category)
			{
				// 해당 재료의 총 수량 누적 합산 (모든 슬롯의 수량 합산)
				int32 TotalAmount = 0;
				for (const FStruct_SlotAddress& SlotAddr : MatAddress.Address)
				{
					TotalAmount += SlotAddr.Amount;
				}

				// 실제로 해당 영역에 존재하는 아이템인 경우에만 생성
				if (TotalAmount > 0)
				{
					// 슬롯 위젯 동적 생성
					APlayerController* PC = GetOwningPlayer();
					UAlchemyCropSlotBase* NewSlot = PC ? CreateWidget<UAlchemyCropSlotBase>(PC, CropSlotClass) : nullptr;
					if (NewSlot)
					{
						NewSlot->ItemID = ItemID;
						NewSlot->Amount = TotalAmount;
						NewSlot->Texture = ItemData->ItemIcon;
						NewSlot->AlchemyInventoryUI = this;
						NewSlot->InitSlot(bInsertedSlot); // 투입된 슬롯(bInsertedSlot = true)이면 드래그 불가 처리

						// 가로 UniformGridPanel에 슬롯 추가 (Row = 0, Column)
						UGP_InventorySlots->AddChildToUniformGrid(NewSlot, 0, Column);
						Slots.Add(NewSlot);
						Column++;
					}
				}
			}
		}
	}
}
