// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ComboWindow.h"

#include "AGSDCharacter.h"

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AAGSDCharacter* Character = Cast<AAGSDCharacter>(MeshComp->GetOwner()))
	{
		Character->SetCanCombo(true);
		// 만약 버퍼된 입력이 있다면 여기서 즉시 다음 콤보 실행 시도 가능
		// (복귀 동작으로 넘어가기 전 연계)
		if (Character->HasBufferedInput())
		{
			Character->ExecuteNextStage();
		}
	}
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AAGSDCharacter* Character = Cast<AAGSDCharacter>(MeshComp->GetOwner()))
	{
		Character->SetCanCombo(false);
	}
}
