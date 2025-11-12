// Fill out your copyright notice in the Description page of Project Settings.


#include "FarmingGameMode.h"
#include "AGSDPlayerController.h"
#include "Kismet/GameplayStatics.h"


AFarmingGameMode::AFarmingGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	GameStateClass = AAGSDGameStateBase::StaticClass();
}

void AFarmingGameMode::BeginPlay()
{
	Super::BeginPlay();
	GameState = GetGameState<AAGSDGameStateBase>();
}

void AFarmingGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleTimeIncrement(DeltaSeconds * TimeScale);
}

void AFarmingGameMode::HandleTimeIncrement(float time)
{
	if (!GameState) return;
	GameState->AddCurrentTime(time);

	if (GameState->GetCurrentTime() >= GameHoursPerDay)
	{
		GameState->InitCurrentTime();
		Reset = true;
	}
	if (GameState->GetCurrentTime() > 360.f && Reset)
	{
		Reset = false;
		GameState->AdvanceDay();
	}
	GameState->OnSecondChangedDelegate.Broadcast(GameState->GetCurrentTime());
}


