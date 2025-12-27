#include "Trigger/ConversationTrigger.h"

#include "Components/BoxComponent.h"
#include "Interface/PetConversationInterface.h"


AConversationTrigger::AConversationTrigger()
{
	// 틱 비활성화
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	
}

void AConversationTrigger::BeginPlay()
{
	Super::BeginPlay();

	if ( TriggerBox )
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AConversationTrigger::OnTriggerBoxBeginOverlap);
	}
}

void AConversationTrigger::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. Actor와 OtherComp가 유효한지 확인
	if (OtherActor && OtherComp && OtherActor->ActorHasTag(TEXT("Pet")))
	{
		// 겹친 컴포넌트(OtherComp)가 액터의 루트 컴포넌트인지 확인
		if (OtherComp == OtherActor->GetRootComponent())
		{
			// 2. 인터페이스 구현 확인
			if (OtherActor->Implements<UPetConversationInterface>())
			{
				// 3. 인터페이스 함수 실행
				IPetConversationInterface::Execute_TriggerPetConversation(OtherActor, DialogueID);
				Destroy(); 
			}
		}
	}
}

