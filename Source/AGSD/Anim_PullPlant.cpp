#include "Anim_PullPlant.h"
#include "AGSDCharacter.h"
#include "Weeds.h"
#include "NiagaraFunctionLibrary.h"

void UAnim_PullPlant::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	AAGSDCharacter* Player = Cast<AAGSDCharacter>(MeshComp->GetOwner());
	if (Player)
	{
		AActor* InteractableActor = Player->getCurrentInteractableActor();
		if (InteractableActor)
		{
			InteractableActor->SetActorHiddenInGame(true);

			if (AWeeds* Weed = Cast<AWeeds>(InteractableActor))
			{
				if (Weed->WeedEffect)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						MeshComp->GetWorld(),
						Weed->WeedEffect,
						Weed->GetActorLocation(),
						Weed->GetActorRotation()
					);
				}
			}
		}
	}
}
