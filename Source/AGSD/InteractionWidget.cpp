// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"

#include "Components/TextBlock.h"

void UInteractionWidget::PlayFadeInAnim(bool IsReverse)
{
	if (!FadeInAnim) return;

	UnbindAllFromAnimationFinished(FadeInAnim);
	
	if (!IsReverse)
	{
		PlayAnimation(FadeInAnim);
	}
	else
	{
		PlayAnimationReverse(FadeInAnim);
		FWidgetAnimationDynamicEvent EndDelegate;
		EndDelegate.BindDynamic(this, &UInteractionWidget::OnFadeOutAnimFinished);
		BindToAnimationFinished(FadeInAnim, EndDelegate);
	}
}

void UInteractionWidget::OnFadeOutAnimFinished()
{
	RemoveFromParent();
}

void UInteractionWidget::SetInteractionText(const FText& NewText)
{
	if (InteractionText)
	{
		InteractionText->SetText(NewText);
	}
}
