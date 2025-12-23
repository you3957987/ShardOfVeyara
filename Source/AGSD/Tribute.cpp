// Fill out your copyright notice in the Description page of Project Settings.


#include "Tribute.h"

#include "AGSDCharacter.h"
#include "TributeUI.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
ATribute::ATribute()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	TributeUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("TributeUI"));
	TributeUI->SetupAttachment(RootComponent);
	TributeUI->SetWidgetSpace(EWidgetSpace::Screen);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATribute::OnBeginOverlap);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ATribute::OnEndOverlap);
}

// Called when the game starts or when spawned
void ATribute::BeginPlay()
{
	Super::BeginPlay();

	if (!TributeUI) return;
	TributeUIInstance = Cast<UTributeUI>(TributeUI->GetUserWidgetObject());
	if (!TributeUIInstance || !TributeDataTable) return;
	SetNextTributeUI();
}

void ATribute::SetNextTributeUI()
{
	FTributeData* CurrentLevelRow = TributeDataTable->FindRow<FTributeData>(FName(*FString::FromInt(++TributeLevel)), TEXT("ContextString"));
	if (CurrentLevelRow)
	{
		CurrentLevelTributeItems = CurrentLevelRow->TributeItems;
	}
	TributeUIInstance->SetNextTributeItem(CurrentLevelTributeItems);
}

// Called every frame
void ATribute::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATribute::Interact_Implementation(AAGSDCharacter* player)
{
	PlayFireNiagara();
	FString ItemID = player->SubItemAmount();
	int* FoundAmount = CurrentLevelTributeItems.Find(ItemID);
	if (FoundAmount)
	{
		(*FoundAmount)--;
	}
	TributeUIInstance->SetNextTributeItem(CurrentLevelTributeItems);

	bool bIsLevelComplete = true;
	for (auto& Elem : CurrentLevelTributeItems)
	{
		if (Elem.Value > 0)
		{
			bIsLevelComplete = false;
			break;
		}
	}

	if (bIsLevelComplete)
	{
		CurrentLevelTributeItems = {};
		PlayFireExplosionNiagara();
		player->AddDamage(10.0f);
		SetNextTributeUI();
	}
}

void ATribute::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool ATribute::CanInteract_Implementation(AAGSDCharacter* player)
{
	if (int* FoundAmount = CurrentLevelTributeItems.Find(player->getPlayerHoidingItemID()))
	{
		if (*FoundAmount > 0) return true;
	}
	return false;
}

void ATribute::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		TributeUIInstance->SetTargetOpacity(1.0f);
		player->AddInteractableActor(this);
	}
}

void ATribute::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		TributeUIInstance->SetTargetOpacity(0.0f);
		player->RemoveInteractableActor(this);
	}
}

