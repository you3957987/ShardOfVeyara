// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim_PullPlant.h"

#include "AGSDCharacter.h"

void UAnim_PullPlant::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	AAGSDCharacter* Player = Cast<AAGSDCharacter>(MeshComp->GetOwner());
	if (Player)
	{
		Player->getCurrentInteractableActor()->SetActorHiddenInGame(true);
	}
}
