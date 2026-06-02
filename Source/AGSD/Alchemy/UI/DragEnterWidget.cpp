// Fill out your copyright notice in the Description page of Project Settings.


#include "DragEnterWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "AlchemyCropSlotBase.h"

void UDragEnterWidget::NativeOnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation)
{
	Super::NativeOnDragEnter(MyGeometry, DragDropEvent, Operation);

	if (Operation && Cast<UAlchemyCropSlotBase>(Operation->Payload))
	{
		bHovered = true;
		
		UE_LOG(LogTemp, Warning, TEXT("DragEnterWidget: Hovered"));
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("Hovered"));
		}
#endif
	}
}

void UDragEnterWidget::NativeOnDragLeave(const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation)
{
	Super::NativeOnDragLeave(DragDropEvent, Operation);

	if (Operation && Cast<UAlchemyCropSlotBase>(Operation->Payload))
	{
		bHovered = false;

		UE_LOG(LogTemp, Warning, TEXT("DragEnterWidget: UnHovered"));
#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("UnHovered"));
		}
#endif
	}
}
