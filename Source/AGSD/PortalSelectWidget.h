// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PortalSelectWidget.generated.h"

/**
 * 
 */
UCLASS()
class AGSD_API UPortalSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	class ALevelSelectPortal* LevelSelectPortal;

	FORCEINLINE class UButton* getCloseButton() { return CloseButton; }

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION()
	void OnSelectButtonClicked();
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* CloseButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* SelectButton;
	
};
