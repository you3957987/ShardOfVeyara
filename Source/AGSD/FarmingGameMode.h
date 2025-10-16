// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AGSDGameStateBase.h"
#include "FarmingGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AGSD_API AFarmingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFarmingGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float TimeScale = 12.f;

	const float GameHoursPerDay = 24.0f;

	class AAGSDGameStateBase* GameState = nullptr;
};
