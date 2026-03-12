// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameSlot.h"

#include "SOVGameInstance.h"
#include "SOVSaveGame.h"
#include "Components/Button.h"
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
			if (SlotButton)
			{
				SlotButton->OnClicked.AddDynamic(this, &USaveGameSlot::OnClickSavedSlotButton);
			}
			bIsEmpty = false;
			EmptyText->SetVisibility(ESlateVisibility::Collapsed);

			if (PlayMap)
			{
				
			}
			if (PlayDay)
			{
				FString DayString = TEXT("Day ");
				DayString += FString::Printf(TEXT("%d"), LoadedInstance->SaveData.CurrentDay);
				PlayDay->SetText(FText::FromString(DayString));
			}	
			if (PlayTime)
			{
				FString TimeString;
				int CorrectionTime = FMath::FloorToInt(LoadedInstance->SaveData.CurrentTime);
				if (CorrectionTime / 60 <= 12 || CorrectionTime / 60 >= 24)
				{
					TimeString = TEXT("AM ");
					TimeString += FString::Printf(TEXT("%02d:"), CorrectionTime / 60);
				}
				else
				{
					TimeString = TEXT("PM ");
					if ((CorrectionTime - 12 * 60) / 60 == 0)
					{
						TimeString += FString::Printf(TEXT("%02d:"), 12);
					}
					else TimeString += FString::Printf(TEXT("%02d:"), (CorrectionTime - 12 * 60) / 60);
				}
				TimeString += FString::Printf(TEXT("%02d"), CorrectionTime % 60);
				PlayTime->SetText(FText::FromString(TimeString));
			}
			if (LastSaveTime)
			{
				FString LastSaveTimeString = LoadedInstance->SaveData.LastSaveTime.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
				LastSaveTime->SetText(FText::FromString(LastSaveTimeString));
			}
		}
		
		return;
	}

	SaveDataPanel->SetVisibility(ESlateVisibility::Collapsed);
	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &USaveGameSlot::OnClickEmptySlotButton);
	}
}

void USaveGameSlot::OnClickEmptySlotButton()
{
	if (USOVGameInstance* GI = Cast<USOVGameInstance>(GetGameInstance()))
	{
		GI->SaveGameSlot = SlotName;
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName("Tutorial_Village"));
}

void USaveGameSlot::OnClickSavedSlotButton()
{
	if (USOVGameInstance* GI = Cast<USOVGameInstance>(GetGameInstance()))
	{
		GI->SaveGameSlot = SlotName;
		GI->LoadGame();
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName("Farm_Sky_Island"));
}


