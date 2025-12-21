// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SOVGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class AGSD_API USOVGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPlayerHealth = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerHealth = MaxPlayerHealth;
};
