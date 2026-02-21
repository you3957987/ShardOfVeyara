// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalSelectWidget.h"

#include "EngineUtils.h"
#include "LevelSelectPortal.h"
#include "ShardsAltar.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPortalSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//버튼이 있을 때 클릭 시 이벤트 바인딩
	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UPortalSelectWidget::OnCloseButtonClicked);
	if (SelectButton)
		SelectButton->OnClicked.AddDynamic(this, &UPortalSelectWidget::OnSelectButtonClicked);
	if (NextMapButton)
		NextMapButton->OnClicked.AddDynamic(this, &UPortalSelectWidget::OnNextMapButtonClicked);
	if (PrevMapButton)
		PrevMapButton->OnClicked.AddDynamic(this, &UPortalSelectWidget::OnPrevMapButtonClicked);
	
	if (LevelSelectPortal)
	{
		mapStruct = LevelSelectPortal->getMapStruct(index);
	}
	if (!altar)
	{
		for (TActorIterator<AShardsAltar> It(GetWorld()); It; ++It)
		{
			altar = *It;
		}
	}
}

void UPortalSelectWidget::OnCloseButtonClicked()
{
	//위젯이 닫힐 때 바인딩 해제
	CloseButton->OnClicked.RemoveDynamic(this, &UPortalSelectWidget::OnCloseButtonClicked);
	SelectButton->OnClicked.RemoveDynamic(this, &UPortalSelectWidget::OnSelectButtonClicked);
	NextMapButton->OnClicked.RemoveDynamic(this, &UPortalSelectWidget::OnNextMapButtonClicked);
	PrevMapButton->OnClicked.RemoveDynamic(this, &UPortalSelectWidget::OnPrevMapButtonClicked);
	
	LevelSelectPortal->OnWidgetClosed();
	//위젯 제거
	RemoveFromParent();
}

void UPortalSelectWidget::OnSelectButtonClicked()
{
	SetTeleTag(mapStruct.TeleTag);
	// IsNull() 대신 경로가 실제로 존재하는지 체크
	FString PathName = mapStruct.Level.GetLongPackageName();

	// 경로가 "None"이 아니고 비어있지 않다면 실행
	if (!PathName.IsEmpty() && PathName != TEXT("None"))
	{
		UGameplayStatics::OpenLevel(GetWorld(), FName(*PathName), true);
	}
}

void UPortalSelectWidget::OnNextMapButtonClicked()
{
	if(MapImage)
	{
		if (LevelSelectPortal)
		{
			index = (index + 1) % (altar->getShardsAmount() + 1);
			mapStruct = LevelSelectPortal->getMapStruct(index);
		}
		MapImage->SetBrushFromTexture(mapStruct.Texture);
		MapName->SetText(mapStruct.MapName);
	}
}

void UPortalSelectWidget::OnPrevMapButtonClicked()
{
	if(MapImage)
	{
		if (LevelSelectPortal)
		{
			index = (LevelSelectPortal->getMapNum() + (index - 1)) % (altar->getShardsAmount() + 1);
			mapStruct = LevelSelectPortal->getMapStruct(index);
		}
		MapImage->SetBrushFromTexture(mapStruct.Texture);
		MapName->SetText(mapStruct.MapName);
	}
}
