// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TributeItem.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "TributeUI.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FTributeData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	TMap<FString, int32> TributeItems;

	FTributeData() : TributeItems({}) {}
};

UCLASS()
class AGSD_API UTributeUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<class UTributeItem*> TributeItems;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	TSubclassOf<UTributeItem> TributeItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	float FadeSpeed = 10.0f;

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tribute")
	void SetTributeItemData(const FString& ItemID, int32 Amount, UTributeItem* TargetWidget);

private:
	float TargetOpacity = 0.0f;

public:
	void PlayFadeIn();

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	FORCEINLINE UButton* getCloseButton() const {return CloseButton;};
	
	UFUNCTION()
	FORCEINLINE void setOwnerActor(AActor* owner) {OwnerActor = owner;};
	
protected:
	virtual void NativeConstruct() override;
	//위젯이 제거될 때 호출되는 엔진 함수 오버라이드
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* CloseButton;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeinAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* OwnerActor;
};