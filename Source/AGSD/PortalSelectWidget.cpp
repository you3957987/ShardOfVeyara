// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSelectWidget.h"

#include "LevelSelectPortal.h"
#include "Components/Button.h"

void UPortalSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//버튼이 있을 때 클릭 시 이벤트 바인딩
	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UPortalSelectWidget::OnCloseButtonClicked);
	if (SelectButton)
		SelectButton->OnClicked.AddDynamic(this, &UPortalSelectWidget::OnSelectButtonClicked);
}

void UPortalSelectWidget::OnCloseButtonClicked()
{
	//위젯이 닫힐 때 바인딩 해제
	CloseButton->OnClicked.RemoveDynamic(this, &UPortalSelectWidget::OnCloseButtonClicked);
	SelectButton->OnClicked.RemoveDynamic(this, &UPortalSelectWidget::OnSelectButtonClicked);

	LevelSelectPortal->OnWidgetClosed();
	//위젯 제거
	RemoveFromParent();
}

void UPortalSelectWidget::OnSelectButtonClicked()
{
	
}
