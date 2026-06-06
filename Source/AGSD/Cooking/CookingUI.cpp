// Fill out your copyright notice in the Description page of Project Settings.


#include "Cooking/CookingUI.h"
#include "AGSDCharacter.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Engine/DataTable.h"
#include "Blueprint/DragDropOperation.h"
#include "Alchemy/UI/AlchemyCropSlotBase.h"
#include "Alchemy/UI/AlchemyInventoryUI.h"
#include "Alchemy/PotionDataTable.h"
#include "Inventory/AGSDInventoryComponent.h"
#include "Inventory/UI/AGSDItemNotificationWidget.h"
#include "Struct_ItemData.h"

// ── UCookHotbarSlot 구현 ──

bool UCookHotbarSlot::NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation)
{
	if (Operation)
	{
		// 드래그 페이로드가 AlchemyCropSlotBase 타입인지 확인
		if (UAlchemyCropSlotBase* DroppedSlot = Cast<UAlchemyCropSlotBase>(Operation->Payload))
		{
			FString ItemID = DroppedSlot->ItemID;
			if (!ItemID.IsEmpty())
			{
				CurrentItemID = ItemID;
				OnItemDropped(ItemID); // 블루프린트 연동 이벤트 호출

				// 위젯 트리를 거슬러 올라가 UCookingUI를 찾음
				UWidget* TempWidget = GetParent();
				UCookingUI* CookingUI = nullptr;
				while (TempWidget)
				{
					CookingUI = Cast<UCookingUI>(TempWidget);
					if (CookingUI) break;
					TempWidget = TempWidget->GetParent();
				}

				if (CookingUI)
				{
					CookingUI->HandleCropDroppedIntoSlot(SlotIndex, ItemID);
				}
				return true;
			}
		}
	}

	return Super::NativeOnDrop(MyGeometry, DragDropEvent, Operation);
}

// ── UCookingUI 구현 ──

void UCookingUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	// 1. Farmer 캐릭터 가져오기 및 UI 등록
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AAGSDCharacter* Character = Cast<AAGSDCharacter>(PC->GetPawn()))
		{
			Farmer = Character;
			Character->RegisterCloseableUI(this);
		}
	}

	// 2. 닫기 버튼 이벤트 바인딩
	if (BTN_Close)
	{
		BTN_Close->OnClicked.RemoveDynamic(this, &UCookingUI::OnCloseButtonClicked);
		BTN_Close->OnClicked.AddDynamic(this, &UCookingUI::OnCloseButtonClicked);
	}

	// 3. 10개의 슬롯 위젯 배열 등록
	InsertedCropSlotArray.Empty();
	if (WBP_CookHotbar_1) InsertedCropSlotArray.Add(WBP_CookHotbar_1);
	if (WBP_CookHotbar_2) InsertedCropSlotArray.Add(WBP_CookHotbar_2);
	if (WBP_CookHotbar_3) InsertedCropSlotArray.Add(WBP_CookHotbar_3);
	if (WBP_CookHotbar_4) InsertedCropSlotArray.Add(WBP_CookHotbar_4);
	if (WBP_CookHotbar_5) InsertedCropSlotArray.Add(WBP_CookHotbar_5);
	if (WBP_CookHotbar_6) InsertedCropSlotArray.Add(WBP_CookHotbar_6);
	if (WBP_CookHotbar_7) InsertedCropSlotArray.Add(WBP_CookHotbar_7);
	if (WBP_CookHotbar_8) InsertedCropSlotArray.Add(WBP_CookHotbar_8);
	if (WBP_CookHotbar_9) InsertedCropSlotArray.Add(WBP_CookHotbar_9);
	if (WBP_CookHotbar_10) InsertedCropSlotArray.Add(WBP_CookHotbar_10);

	// 인덱스 값 지정
	for (int32 i = 0; i < InsertedCropSlotArray.Num(); ++i)
	{
		if (InsertedCropSlotArray[i])
		{
			InsertedCropSlotArray[i]->SlotIndex = i;
		}
	}

	// 4. 인벤토리 재료 데이터 갱신
	if (Farmer)
	{
		MaterialAddress = UAlchemyInventoryUI::ScanInventoryForMaterials(Farmer);
	}
}

void UCookingUI::NativeDestruct()
{
	if (Farmer)
	{
		Farmer->UnregisterCloseableUI(this);
	}

	Super::NativeDestruct();
}

void UCookingUI::OnCloseButtonClicked()
{
	IUIClosable::Execute_CloseUI(this);
}

void UCookingUI::CloseUI_Implementation()
{
	RemoveFromParent();
}

void UCookingUI::HandleCropDroppedIntoSlot(int32 SlotIndex, const FString& ItemID)
{
	if (!Farmer || !Farmer->InventoryComponent) return;

	// 인벤토리 상태 최신화
	MaterialAddress = UAlchemyInventoryUI::ScanInventoryForMaterials(Farmer);

	// 인벤토리에서 아이템 주소를 찾음
	FStruct_MaterialAddress* FoundAddr = MaterialAddress.Find(ItemID);
	if (FoundAddr && FoundAddr->Address.Num() > 0)
	{
		int32 TargetSlotIndex = FoundAddr->Address[0].Index;
		
		// 실제 플레이어 가방에서 아이템 수량 1개 차감
		if (Farmer->InventoryComponent->RemoveItem(TargetSlotIndex, 1))
		{
			// UI 내부 인벤토리 주소 데이터도 업데이트
			MaterialAddress = UAlchemyInventoryUI::ScanInventoryForMaterials(Farmer);

			// 레시피 매칭 및 제작 로직 수행
			CheckRecipesAndBrew();
		}
	}
}

void UCookingUI::CheckRecipesAndBrew()
{
	if (!Farmer || !Farmer->InventoryComponent || !RecipeDataTable) return;

	// 1. 현재 핫바 슬롯에서 채워진 작물 아이템 ID들만 수집
	TArray<FString> ActiveIngredients;
	TArray<UCookHotbarSlot*> ActiveSlots;

	for (UCookHotbarSlot* Slot : InsertedCropSlotArray)
	{
		if (Slot && !Slot->CurrentItemID.IsEmpty())
		{
			ActiveIngredients.Add(Slot->CurrentItemID);
			ActiveSlots.Add(Slot);
		}
	}

	// 재료가 정확히 2개 투입되었을 때만 레시피를 검증 (물약 제작 구조와 동일)
	if (ActiveIngredients.Num() == 2)
	{
		// 조합 대조를 위해 재료 배열 정렬
		ActiveIngredients.Sort();

		// 레시피 테이블에서 행 목록 가져오기
		TArray<FPotionData*> Recipes;
		static const FString ContextString(TEXT("Cooking Recipe Match Context"));
		RecipeDataTable->GetAllRows<FPotionData>(ContextString, Recipes);

		for (FPotionData* Recipe : Recipes)
		{
			if (!Recipe) continue;

			// 레시피 재료 정렬
			TArray<FString> RecipeIngredients = { Recipe->IngredientA, Recipe->IngredientB };
			RecipeIngredients.Sort();

			// 투입된 재료 목록이 레시피와 일치하면 제작 성공
			if (ActiveIngredients == RecipeIngredients)
			{
				FString PotionItemID = Recipe->ItemID;
				if (PotionItemID.IsEmpty())
				{
					PotionItemID = TEXT("Sludge");
				}

				// 완성 포션(요리) 지급
				int32 OutRemainingQty = 0;
				FStruct_ItemData OutItemData;
				Farmer->InventoryComponent->AddItemByID(PotionItemID, 1, OutRemainingQty, OutItemData);

				// 요리 획득 화면 알림 표시
				AddItemNotification(PotionItemID);

				// 사용된 슬롯 초기화 및 비주얼 리셋 호출
				for (UCookHotbarSlot* Slot : ActiveSlots)
				{
					Slot->CurrentItemID = TEXT("");
					Slot->OnItemDropped(TEXT(""));
				}

				break; // 레시피 매칭 및 완성 후 루프 탈출
			}
		}
	}
}

void UCookingUI::AddItemNotification(const FString& ItemID, int32 Amount)
{
	if (!VB_ItemNotificationList || !ItemNotificationClass || !ItemDataTable || !Farmer) return;

	APlayerController* PC = Cast<APlayerController>(Farmer->GetController());
	if (!PC) return;

	FStruct_ItemData* ItemData = ItemDataTable->FindRow<FStruct_ItemData>(FName(*ItemID), TEXT("CookingItemNotification"));
	if (!ItemData) return;

	UAGSDItemNotificationWidget* NotiWidget = CreateWidget<UAGSDItemNotificationWidget>(PC, ItemNotificationClass);
	if (NotiWidget)
	{
		NotiWidget->SetupNotification(ItemData->ItemName.ToString(), Amount, ItemData->ItemIcon);
		VB_ItemNotificationList->AddChild(NotiWidget);
	}
}
