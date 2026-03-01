// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlotSaveData.h"
#include "Struct_InventorySlotData.h"
#include "Struct_WorldChestData.h"
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
	USOVGameInstance();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame")
	FString SaveGameSlot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldState")
	TArray<FString> NoRegenItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldState")
	FName TeleportationTag = FName("None");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldState")
	int CurrentDay = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldState")
	float CurrentTime = 360.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldState")
	TMap<FString, FStruct_WorldChestData> ChestMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	float MaxPlayerHealth = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	float PlayerHealth = MaxPlayerHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	bool bHasPet = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	int32 CurrentSelectedHotbar = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	TArray<FStruct_InventorySlotData> TempInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	TArray<FStruct_InventorySlotData> TempHotbar;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FarmingState")
	TMap<FString, FPlotSaveData> GlobalPlotDataMap;

	// 경작지로부터 데이터를 받아서 저장/갱신하는 함수
	UFUNCTION(BlueprintCallable, Category = "Farming")
	void UpdatePlotData(FPlotSaveData NewData);
    
	// GUID로 데이터를 꺼내주는 함수 (나중에 로드할 때 씀)
	UFUNCTION(BlueprintCallable, Category = "Farming")
	bool GetPlotData(FString name, FPlotSaveData& OutData);
};
