// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameSlot.h"

#include "SOVGameInstance.h"
#include "SOVSaveGame.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "ConfirmationDialog.h"
#include "Struct_MapData.h"
#include "Components/Image.h"

void USaveGameSlot::NativeConstruct()
{
	if (SlotNum != nullptr)
	{
		SlotNum->SetText(FText::AsNumber(SlotIndex));
	}
	if (DeleteButton != nullptr)
	{
		DeleteButton->OnClicked.AddDynamic(this, &USaveGameSlot::OnClickDeleteButton);
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

			FName LevelName = FName(*LoadedInstance->SaveData.Level.GetAssetName());
			static const FString ContextString(TEXT("Item Look Up Context"));
			FStruct_MapData* Row = MapDataTable->FindRow<FStruct_MapData>(LevelName, ContextString);
			if (PlayMapImage && Row)
			{
				PlayMapImage->SetBrushFromTexture(Row->MapImage);
			}
			if (PlayMap && Row)
			{
				PlayMap->SetText(Row->MapName);
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
		FName LevelName = FName(*GI->Level.GetAssetName());
    
		UGameplayStatics::OpenLevel(this, LevelName);
	}
}

void USaveGameSlot::OnClickDeleteButton()
{
	if (!ConfirmationDialog && ConfirmationDialogClass)
	{
		ConfirmationDialog = CreateWidget<UConfirmationDialog>(GetWorld(), ConfirmationDialogClass);
	}
	if (ConfirmationDialog)
	{
		ConfirmationDialog->YesButton->OnClicked.AddDynamic(this, &USaveGameSlot::OnDeleteSlot);
		ConfirmationDialog->AddToViewport();
	}
}

void USaveGameSlot::OnDeleteSlot()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		// 2. 세이브 파일 삭제
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);
	}
	
	SaveDataPanel->SetVisibility(ESlateVisibility::Collapsed);
	EmptyText->SetVisibility(ESlateVisibility::Visible);
	bIsEmpty = true;
	
	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &USaveGameSlot::OnClickEmptySlotButton);
	}
	if (ConfirmationDialog)
	{
		ConfirmationDialog->YesButton->OnClicked.RemoveAll(this);
		ConfirmationDialog->RemoveFromParent();
	}
}

void USaveGameSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!bIsEmpty && DeleteButtonPanel)
	{
		DeleteButtonPanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void USaveGameSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (DeleteButtonPanel)
	{
		DeleteButtonPanel->SetVisibility(ESlateVisibility::Hidden);
	}
}


