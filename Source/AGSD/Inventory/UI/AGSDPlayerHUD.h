#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Struct_ItemData.h"
#include "AGSDPlayerHUD.generated.h"

class UAGSDHotbarWidget;
class UAGSDInventoryWidget;
class UAGSDInventoryComponent;
class UVerticalBox;
class UAGSDItemNotificationWidget;

/**
 * UAGSDPlayerHUD
 * 인게임 전체 HUD 컨테이너 위젯의 C++ 베이스 클래스입니다.
 * 단축 핫바 및 메인 가방 인벤토리 UI를 보유하며, 개폐 및 마우스 포커스 관리를 수행합니다.
 */
UCLASS()
class AGSD_API UAGSDPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── UI 바인딩 ──
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UAGSDHotbarWidget> WBP_Hotbar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UAGSDInventoryWidget> WBP_InventoryUI;

	/** 아이템 획득 알림들이 쌓이는 수직 박스 컨테이너 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI")
	TObjectPtr<UVerticalBox> VB_ItemNotificationList;

	// ── 애니메이션 ──
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> HotbarSlide;

	// ── 참조 ──
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UAGSDInventoryComponent> InventoryComponent;

	// ── 설정 데이터 ──
	/** 스폰할 아이템 획득 알림 위젯의 C++ / Blueprint 템플릿 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player HUD|Config")
	TSubclassOf<UAGSDItemNotificationWidget> ItemNotificationClass;

	// ── 기능 함수 ──

	/** HUD에 인벤토리 컴포넌트를 연결하고 하위 위젯들을 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void InitializeHUD(UAGSDInventoryComponent* InInventoryComponent);

	/** 인벤토리 UI 창의 개폐를 토글합니다. (입력 모드 전환 포함) */
	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void ToggleInventory();

	/** 인벤토리 UI 창을 강제로 닫습니다. */
	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void CloseInventory();

	/** 인벤토리 UI 창이 현재 열려있는지 여부를 반환합니다. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player HUD")
	bool IsInventoryOpen() const;

	/** 새로운 아이템 획득 알림 위젯을 생성하여 알림 리스트에 추가합니다. */
	UFUNCTION(BlueprintCallable, Category = "Player HUD")
	void AddItemNotification(FStruct_ItemData ItemData, int32 Amount);

protected:
	virtual void NativeConstruct() override;
};

