// Fill out your copyright notice in the Description page of Project Settings.


#include "FarmingGameMode.h"
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
	if (!GameState)
	{
		UE_LOG(LogTemp, Error, TEXT("Game State is null"));
	}
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


