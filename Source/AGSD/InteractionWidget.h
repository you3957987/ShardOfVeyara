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
	
protected:
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeInAnim;

	UFUNCTION()
	void OnFadeOutAnimFinished();
};
