// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class AGSD_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void PlayFadeInAnim(bool IsReverse);

	UFUNCTION()
	void SetInteractionText(const FText& NewText);
	
protected:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeInAnim;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InteractionText;
	
	UFUNCTION()
	void OnFadeOutAnimFinished();
};
