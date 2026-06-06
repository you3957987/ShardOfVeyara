// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AGSDCloseableUIInterface.h"
#include "Alchemy/Struct_MaterialAddress.h"
#include "CookingUI.generated.h"

class UButton;
class UVerticalBox;
class UDataTable;
class AAGSDCharacter;
class UAGSDItemNotificationWidget;

/**
 * UCookHotbarSlot
 * 요리 UI의 개별 재료 슬롯 칸을 제어하는 C++ 클래스입니다.
 */
UCLASS()
class AGSD_API UCookHotbarSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking", meta = (ExposeOnSpawn = "true"))
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Cooking")
	FString CurrentItemID = TEXT("");

protected:
	virtual bool NativeOnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation) override;

	// 블루프린트에서 비주얼 처리(예: 작물 이미지 표시 등)를 오버라이드할 수 있도록 이벤트 제공
	UFUNCTION(BlueprintImplementableEvent, Category = "Cooking")
	void OnItemDropped(const FString& ItemID);
};

/**
 * UCookingUI
 * 요리(Cooking) UI 전체 화면을 제어하고, 레시피 조합 및 요리 생성을 담당하는 C++ 부모 클래스입니다.
 */
UCLASS()
class AGSD_API UCookingUI : public UUserWidget, public IUIClosable
{
	GENERATED_BODY()

public:
	// IUIClosable 인터페이스 구현
	virtual void CloseUI_Implementation() override;

	// 슬롯에 작물이 정상 드롭되었을 때 처리하는 핸들러
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void HandleCropDroppedIntoSlot(int32 SlotIndex, const FString& ItemID);

	// 현재 투입된 작물들로 레시피를 검사하고 요리를 생성하는 메인 로직
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void CheckRecipesAndBrew();

	// 물약 획득 시 알림 위젯 출력
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void AddItemNotification(const FString& ItemID, int32 Amount = 1);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCloseButtonClicked();

	// ── UI 바인딩 ──
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BTN_Close;

	// WBP_CookHotbar 1~10 위젯 바인딩 (UCookHotbarSlot* 타입으로 연동)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_5;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_6;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_7;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_8;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_9;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCookHotbarSlot> WBP_CookHotbar_10;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VB_ItemNotificationList;

	// ── 설정 데이터 테이블 ──
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking")
	TObjectPtr<UDataTable> RecipeDataTable; // 연금술 레시피 데이터 테이블 (FPotionData 행)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking")
	TObjectPtr<UDataTable> ItemDataTable; // 아이템 정보 데이터 테이블 (FStruct_ItemData 행)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking")
	TSubclassOf<UAGSDItemNotificationWidget> ItemNotificationClass;

	// ── 내부 관리 변수 ──
	UPROPERTY(BlueprintReadOnly, Category = "Cooking")
	TObjectPtr<AAGSDCharacter> Farmer;

	UPROPERTY(BlueprintReadOnly, Category = "Cooking")
	TArray<TObjectPtr<UCookHotbarSlot>> InsertedCropSlotArray;

	// 인벤토리 재료 캐싱 주소 데이터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking")
	TMap<FString, FStruct_MaterialAddress> MaterialAddress;
};
