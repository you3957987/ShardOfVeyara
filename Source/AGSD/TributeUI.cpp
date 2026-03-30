// Fill out your copyright notice in the Description page of Project Settings.


#include "TributeUI.h"

#include "Components/Button.h"

void UTributeUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UTributeUI::OnCloseButtonClicked);
	}
}

void UTributeUI::PlayFadeIn()
{
	if (FadeinAnim)
	{
		PlayAnimation(FadeinAnim);
	}
}

void UTributeUI::NativeDestruct()
{
	Super::NativeDestruct();
}

void UTributeUI::OnCloseButtonClicked()
{
	// 스스로를 화면에서 제거합니다.
	RemoveFromParent();
}