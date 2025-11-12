// Fill out your copyright notice in the Description page of Project Settings.


#include "ClockWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AGSDGameStateBase.h"
#include "Components/TextBlock.h"

bool UClockWidget::Initialize()
{
	if (!Super::Initialize()) return false;
	
	if (AAGSDGameStateBase* GameState = Cast<AAGSDGameStateBase>(UGameplayStatics::GetGameState(GetWorld())))
	{
		GameState->OnSecondChangedDelegate.AddDynamic(this, &UClockWidget::SetTimeText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState Cast failed in UClockWidget::Initialize"))
	}
	
	return true;
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
