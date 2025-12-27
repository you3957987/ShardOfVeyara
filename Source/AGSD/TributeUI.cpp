// Fill out your copyright notice in the Description page of Project Settings.


#include "TributeUI.h"

#include "Components/UniformGridPanel.h"
#include "Dataflow/DataflowContent.h"

void UTributeUI::SetNextTributeItem(const TMap<FString, int32>& ItemMap)
{
	TArray<FString> ItemKeys;
	ItemMap.GetKeys(ItemKeys);
	
	int32 RequiredCount = ItemKeys.Num();
	int32 CurrentCount = TributeItems.Num();

	for (int32 i = 0; i < RequiredCount; i++)
	{
		UTributeItem* TargetWidget = nullptr;

		if (i < CurrentCount)
		{
			TargetWidget = TributeItems[i];
		}
		else
		{
			TargetWidget = CreateWidget<UTributeItem>(this, TributeItem);
			if (TargetWidget)
			{
				TributeItems.Add(TargetWidget);
				if (TributeItemGrid)
				{
					TributeItemGrid->AddChildToUniformGrid(TargetWidget, 0, i);
				}
			}
		}
		if (TargetWidget)
		{
			FString CurrentID = ItemKeys[i];
			int32 CurrentAmount = ItemMap[CurrentID];
			
			SetTributeItemData(CurrentID, CurrentAmount, TargetWidget);
			TargetWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}

	for (int32 i = RequiredCount; i < TributeItems.Num(); i++)
	{
		TributeItems[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UTributeUI::SetTargetOpacity(float NewOpacity)
{
	TargetOpacity = FMath::Clamp(NewOpacity, 0.0f, 1.0f);
}

void UTributeUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	float CurrentOpacity = GetRenderOpacity();

	// 1. [보간] 현재 투명도에서 목표 투명도로 부드럽게 이동합니다.
	// FInterpTo는 Target에 가까워질수록 속도가 줄어들어 자연스러운 감속 효과를 줍니다.
	float NextOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, InDeltaTime, FadeSpeed);

	// Render Opacity 적용
	SetRenderOpacity(NextOpacity);
}

void UTributeUI::NativeConstruct()
{
	Super::NativeConstruct();

	SetRenderOpacity(0.0f);
}