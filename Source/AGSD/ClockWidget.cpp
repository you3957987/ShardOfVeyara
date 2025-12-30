// Fill out your copyright notice in the Description page of Project Settings.


#include "ClockWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AGSDGameStateBase.h"
#include "SOVGameInstance.h"
#include "Components/TextBlock.h"

void UClockWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UWorld* World = GetWorld();
	if (!World) return;
	
	if (AAGSDGameStateBase* GameState = Cast<AAGSDGameStateBase>(UGameplayStatics::GetGameState(World)))
	{
		GameState->OnSecondChangedDelegate.AddDynamic(this, &UClockWidget::SetTimeText);
		GameState->OnDayChangedDelegate.AddUObject(this, &UClockWidget::SetDayText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState Cast failed in UClockWidget::Initialize"))
	}
}

void UClockWidget::NativeConstruct()
{
	Super::NativeConstruct();
	USOVGameInstance* GI = Cast<USOVGameInstance>(GetGameInstance());
	SetDayText(GI->CurrentDay);
}

void UClockWidget::SetTimeText(float time)
{
	if (TimeText)
	{
		FString TimeString;
		int CorrectionTime = FMath::FloorToInt(time);
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
		TimeText->SetText(FText::FromString(TimeString));
	}
}

void UClockWidget::SetDayText(int32 day)
{
	if (DayText)
	{
		FString DayString = TEXT("Day ");
		DayString += FString::Printf(TEXT("%d"), day);
		DayText->SetText(FText::FromString(DayString));
	}	
}
