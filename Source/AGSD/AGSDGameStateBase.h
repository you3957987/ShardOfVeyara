// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AGSDGameStateBase.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32 /* CurrentDay */);

UCLASS()
class AGSD_API AAGSDGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	FOnDayChanged OnDayChangedDelegate;
	
	void AdvanceDay();

	FORCEINLINE int32 GetCurrentDay() const { return CurrentDay; }
	FORCEINLINE int32 GetCurrentTime() const { return CurrentTime; }
	FORCEINLINE void SetCurrentTime(float Time) { CurrentTime += Time; }
	FORCEINLINE void InitCurrentTime() { CurrentTime = 0.0f; }

	UFUNCTION()
	void OnRep_CurrentDay();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UPROPERTY(visibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 CurrentDay = 1;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly, Category = "Time")
	float CurrentTime = 0.0f;
};
