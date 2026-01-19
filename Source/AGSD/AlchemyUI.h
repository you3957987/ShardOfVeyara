// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlchemyUI.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetClosed);
UCLASS()
class AGSD_API UAlchemyUI : public UUserWidget
{
	GENERATED_BODY()

public:
	//델리게이트 변수 생성
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWidgetClosed OnWidgetClosed;

	void PlayFadeIn();

protected:
	virtual void NativeConstruct() override;
	//위젯이 제거될 때 호출되는 엔진 함수 오버라이드
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* FadeinAnim;

	UFUNCTION()
	void OnCloseButtonClicked();
};
