// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlotSaveData.h"
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
	int CurrentDay = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTime = 360.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPlayerHealth = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerHealth = MaxPlayerHealth;
	/*
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector2D PlayerLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWorld* World;
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPet = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Farming")
	TMap<FString, FPlotSaveData> GlobalPlotDataMap;

	// 경작지로부터 데이터를 받아서 저장/갱신하는 함수
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdatePlotData(FPlotSaveData NewData);
    
	// GUID로 데이터를 꺼내주는 함수 (나중에 로드할 때 씀)
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool GetPlotData(FString name, FPlotSaveData& OutData);
};
