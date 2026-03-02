#pragma once
#include "PlotSaveData.h"
#include "Struct_InventorySlotData.h"
#include "Struct_WorldChestData.h"
#include "SaveData.generated.h"

USTRUCT(BlueprintType)
struct FSaveData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> NoRegenItem;
	
	UPROPERTY()
	int CurrentDay;

	UPROPERTY()
	float CurrentTime;

	UPROPERTY()
	TMap<FString, FStruct_WorldChestData> ChestMap;

	UPROPERTY()
	int Coin;
	
	UPROPERTY()
	float MaxPlayerHealth;
	
	UPROPERTY()
	float PlayerHealth;

	UPROPERTY()
	bool bHasPet;

	UPROPERTY()
	TArray<FStruct_InventorySlotData> TempInventory;

	UPROPERTY()
	TArray<FStruct_InventorySlotData> TempHotbar;
	
	UPROPERTY()
	TMap<FString, FPlotSaveData> GlobalPlotDataMap;

	FSaveData()
	{
		CurrentDay = 1;
		CurrentTime = 360.f;
		Coin = 0;
		MaxPlayerHealth = 200.f;	
		PlayerHealth = MaxPlayerHealth;
		bHasPet = false;
		TempHotbar.SetNum(10);	
	}
};
