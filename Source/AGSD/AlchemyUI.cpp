// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyUI.h"

#include "AlchemyTable.h"
#include "Components/Button.h"

void UAlchemyUI::CallOnCropInserted()
{
	//AlchemyTable->SplashPot();
	OnCropInserted.Broadcast();
}

void UAlchemyUI::PlayFadeIn()
{
	if (FadeinAnim)
	{
		PlayAnimation(FadeinAnim);
	}
}

void UAlchemyUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UAlchemyUI::OnCloseButtonClicked);
	}
}

void UAlchemyUI::NativeDestruct()
{
	Super::NativeDestruct();
}

void UAlchemyUI::OnCloseButtonClicked()
{
	// 스스로를 화면에서 제거합니다.
	RemoveFromParent();
}
