// Fill out your copyright notice in the Description page of Project Settings.


#include "Bed.h"

#include "AGSDCharacter.h"
#include "FarmingGameMode.h"
#include "Components/BoxComponent.h"

// Sets default values
ABed::ABed()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABed::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ABed::OnEndOverlap);
}

void ABed::Interact_Implementation(AAGSDCharacter* player)
{
	player->DisableInput(player->getPlayerController());
	player->RemoveInteractableActor(this);
	if (!WBP_FadeWidget) return;
	if (!FadeWidget) FadeWidget = CreateWidget<UFadeWidget>(player->getPlayerController(), WBP_FadeWidget);
	FadeWidget->SetTargetOpacity(1.f);
	if (!FadeWidget->IsInViewport()) FadeWidget->AddToViewport(100);

	FTimerHandle TimerHandle;
	// 람다(Lambda)를 사용하면 player 정보를 그대로 넘겨줄 수 있어 편리합니다.
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, player]()
	{
		AFarmingGameMode* GM = Cast<AFarmingGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->NextDay(360.f);
		}
		player->EnableInput(player->getPlayerController());
		if (FadeWidget) FadeWidget->SetTargetOpacity(0.f);
	}, FadeTime + 2.0f, false);
}

void ABed::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool ABed::CanInteract_Implementation(AAGSDCharacter* player)
{
	return true;
}

void ABed::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->AddInteractableActor(this);
	}
}

void ABed::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

