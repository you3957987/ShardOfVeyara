// Fill out your copyright notice in the Description page of Project Settings.


#include "SOVGameInstance.h"
#include "SOVSaveGame.h"
#include "Kismet/GameplayStatics.h"

USOVGameInstance::USOVGameInstance()
{
	InitializeVariables();
}

void USOVGameInstance::InitializeVariables()
{
	// 블루프린트 에디터에서 설정한 기본값들이 담긴 원본 객체(CDO)를 가져옵니다.
	const USOVGameInstance* DefaultInstance = GetDefault<USOVGameInstance>(GetClass());
	if (!DefaultInstance) return;

	// 에디터 설정값을 그대로 복사 (Level, 시간, 체력 등)
	NoRegenItem = DefaultInstance->NoRegenItem;
	Level = DefaultInstance->Level;
	TeleportationTag = DefaultInstance->TeleportationTag;
	CurrentDay = DefaultInstance->CurrentDay;
	CurrentTime = DefaultInstance->CurrentTime;
	ShardsAmount = DefaultInstance->ShardsAmount;
	Coin = DefaultInstance->Coin;
	Damage = DefaultInstance->Damage;
	MaxPlayerHealth = DefaultInstance->MaxPlayerHealth;
	PlayerHealth = MaxPlayerHealth;
	bHasPet = DefaultInstance->bHasPet;
	CurrentSelectedHotbar = DefaultInstance->CurrentSelectedHotbar;
	TributeLevel = DefaultInstance->TributeLevel;
	MouseSensitivity = DefaultInstance->MouseSensitivity;
	
	// 데이터 맵 및 인벤토리는 비워줌 (필요 시 CDO에서 복사 가능)
	ChestMap.Empty();
	TempInventory.Empty();
	TempHotbar.Empty();
	TempHotbar.SetNum(10);
	GlobalPlotDataMap.Empty();
	AlchemyTableDataMap.Empty();
	CurrentLevelTributeItems.Empty();
	AlreadyDroppedItems.Empty();
}

void USOVGameInstance::UpdatePlotData(FPlotSaveData NewData)
{
	// 장부에 기록 (이미 있으면 덮어쓰기, 없으면 추가)
	GlobalPlotDataMap.Add(NewData.PlotName, NewData);
}

void USOVGameInstance::UpdateTableData(FAlchemySaveData NewData)
{
	// 장부에 기록 (이미 있으면 덮어쓰기, 없으면 추가)
	AlchemyTableDataMap.Add(NewData.TableName, NewData);
}

bool USOVGameInstance::GetPlotData(FString name, FPlotSaveData& OutData)
{
	if (FPlotSaveData* FoundData = GlobalPlotDataMap.Find(name))
	{
		OutData = *FoundData;
		return true;
	}
	return false;
}

bool USOVGameInstance::GetAlchemyData(FString name, FAlchemySaveData& OutData)
{
	if (FAlchemySaveData* FoundData = AlchemyTableDataMap.Find(name))
	{
		OutData = *FoundData;
		return true;
	}
	return false;
}

FSaveData USOVGameInstance::GetSaveData()
{
	FSaveData SaveData;
	SaveData.NoRegenItem = NoRegenItem;
	SaveData.Level = Level;
	SaveData.TeleportationTag = TeleportationTag;
	SaveData.CurrentDay = CurrentDay;
	SaveData.CurrentTime = CurrentTime;
	SaveData.ChestMap = ChestMap;
	SaveData.ShardsAmount = ShardsAmount;
	SaveData.Coin = Coin;
	SaveData.Damage = Damage;
	SaveData.MaxPlayerHealth = MaxPlayerHealth;
	SaveData.PlayerHealth = PlayerHealth;
	SaveData.bHasPet = bHasPet;
	SaveData.TempInventory = TempInventory;
	SaveData.TempHotbar = TempHotbar;
	SaveData.GlobalPlotDataMap = GlobalPlotDataMap;
	SaveData.AlchemyTableDataMap = AlchemyTableDataMap;
	SaveData.TributeLevel = TributeLevel;
	SaveData.CurrentLevelTributeItems = CurrentLevelTributeItems;
	SaveData.LastSaveTime = FDateTime::Now();
	SaveData.AlreadyDroppedItems = AlreadyDroppedItems;
	SaveData.MouseSensitivity = MouseSensitivity;
	
	return SaveData; 
}

void USOVGameInstance::SetSaveData(FSaveData SaveData)
{
	NoRegenItem = SaveData.NoRegenItem;
	Level = SaveData.Level;
	TeleportationTag = SaveData.TeleportationTag;
	CurrentDay = SaveData.CurrentDay;
	CurrentTime = SaveData.CurrentTime;
	ChestMap = SaveData.ChestMap;
	ShardsAmount = SaveData.ShardsAmount;
	Coin = SaveData.Coin;
	Damage = SaveData.Damage;
	MaxPlayerHealth = SaveData.MaxPlayerHealth;
	PlayerHealth = SaveData.PlayerHealth;
	bHasPet = SaveData.bHasPet;
	TempInventory = SaveData.TempInventory;
	TempHotbar = SaveData.TempHotbar;
	GlobalPlotDataMap = SaveData.GlobalPlotDataMap;
	AlchemyTableDataMap = SaveData.AlchemyTableDataMap;
	TributeLevel = SaveData.TributeLevel;
	CurrentLevelTributeItems = SaveData.CurrentLevelTributeItems;
	AlreadyDroppedItems = SaveData.AlreadyDroppedItems;
	MouseSensitivity = SaveData.MouseSensitivity;
}

void USOVGameInstance::SaveGame()
{
	if (SaveGameSlot.IsEmpty()) return;

	// 세이브 객체 생성
	USOVSaveGame* SaveInstance = Cast<USOVSaveGame>(UGameplayStatics::CreateSaveGameObject(USOVSaveGame::StaticClass()));
    
	if (SaveInstance)
	{
		// 데이터 채우기
		SaveInstance->SaveData = GetSaveData();

		// 파일 쓰기 (동기식)
		UGameplayStatics::SaveGameToSlot(SaveInstance, SaveGameSlot, 0);
	}
}

void USOVGameInstance::LoadGame()
{
	if (SaveGameSlot.IsEmpty()) return;

	// 해당 슬롯에 세이브 파일이 있는지 확인
	if (UGameplayStatics::DoesSaveGameExist(SaveGameSlot, 0))
	{
		// 파일 읽기
		USOVSaveGame* LoadedInstance = Cast<USOVSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveGameSlot, 0));
        
		if (LoadedInstance)
		{
			// 불러온 데이터를 인스턴스 변수에 적용
			SetSaveData(LoadedInstance->SaveData);
		}
	}
}

void USOVGameInstance::Init()
{
	Super::Init();

	// 월드가 정리될 때 호출되는 델리게이트에 HandleWorldCleanup 함수를 연결(Bind)합니다.
	//FWorldDelegates::OnWorldCleanup.AddUObject(this, &USOVGameInstance::HandleWorldCleanup);

	// 2. 월드 초기화가 완료된 후 호출되는 델리게이트에 바인딩 (맵이 열릴 때)
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &USOVGameInstance::HandleWorldInitialized);
}

void USOVGameInstance::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// 실제 게임 월드이거나 에디터 플레이(PIE) 중일 때만 자동 저장 실행
	if (World && (World->IsGameWorld() || World->IsPlayInEditor()))
	{
		// 현재 설정된 스트링(SaveGameSlot)으로 저장 실행
		SaveGame();
	}
}

void USOVGameInstance::HandleWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	// 실제 게임 월드이거나 에디터 플레이(PIE) 중일 때만 자동 저장 실행
	if (World && (World->IsGameWorld() || World->IsPlayInEditor()))
	{
		// 현재 설정된 스트링(SaveGameSlot)으로 저장 실행
		SaveGame();
	}
}
