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

	if (!GameState) return;

	float GameTimeElapsed = DeltaSeconds * TimeScale;
	GameState->SetCurrentTime(GameTimeElapsed);

	if (GameState->GetCurrentTime() >= GameHoursPerDay)
	{
		GameState->InitCurrentTime();
		GameState->AdvanceDay();
	}
}


