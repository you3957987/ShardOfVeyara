// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "AGSDGameStateBase.h"
#include "TimeLight.generated.h"

UCLASS()
class AGSD_API ATimeLight : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATimeLight();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UDirectionalLightComponent* SunLight;

	UPROPERTY(EditAnywhere)
	UDirectionalLightComponent* MoonLight;

	UPROPERTY(EditAnywhere)
	USkyAtmosphereComponent* SkyAtmosphere;

	UPROPERTY(EditAnywhere)
	UExponentialHeightFogComponent* ExponentialHeightFog;

	UPROPERTY(EditAnywhere)
	USkyLightComponent* SkyLight;

	UPROPERTY(EditAnywhere)
	UVolumetricCloudComponent* VolumetricCloud;
	
	AAGSDGameStateBase* GameState;

	UPROPERTY(EditAnywhere)
	bool Timeflow = true;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
