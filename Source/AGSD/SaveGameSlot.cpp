// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameSlot.h"

#include "SOVSaveGame.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USaveGameSlot::NativeConstruct()
{
	if (SlotNum != nullptr)
	{
		SlotNum->SetText(FText::AsNumber(SlotIndex));
	}
	if (SlotName.IsEmpty()) return;

	// 해당 슬롯에 세이브 파일이 있는지 확인
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		// 파일 읽기
		USOVSaveGame* LoadedInstance = Cast<USOVSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
        
		if (LoadedInstance)
		{
			bIsEmpty = false;
			SaveDataPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			EmptyText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}


