// Fill out your copyright notice in the Description page of Project Settings.


#include "SOVGameInstance.h"

#include "SOVSaveGame.h"
#include "Kismet/GameplayStatics.h"

USOVGameInstance::USOVGameInstance()
{
	TempHotbar.SetNum(10);
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
	SaveData.CurrentDay = CurrentDay;
	SaveData.CurrentTime = CurrentTime;
	SaveData.ChestMap = ChestMap;
	SaveData.Coin = Coin;
	SaveData.MaxPlayerHealth = MaxPlayerHealth;
	SaveData.PlayerHealth = PlayerHealth;
	SaveData.bHasPet = bHasPet;
	SaveData.TempInventory = TempInventory;
	SaveData.TempHotbar = TempHotbar;
	SaveData.GlobalPlotDataMap = GlobalPlotDataMap;
	SaveData.AlchemyTableDataMap = AlchemyTableDataMap;
	
	return SaveData; 
}

void USOVGameInstance::SetSaveData(FSaveData SaveData)
{
	NoRegenItem = SaveData.NoRegenItem;
	CurrentDay = SaveData.CurrentDay;
	CurrentTime = SaveData.CurrentTime;
	ChestMap = SaveData.ChestMap;
	Coin = SaveData.Coin;
	MaxPlayerHealth = SaveData.MaxPlayerHealth;
	PlayerHealth = SaveData.PlayerHealth;
	bHasPet = SaveData.bHasPet;
	TempInventory = SaveData.TempInventory;
	TempHotbar = SaveData.TempHotbar;
	GlobalPlotDataMap = SaveData.GlobalPlotDataMap;
	AlchemyTableDataMap = SaveData.AlchemyTableDataMap;
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
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &USOVGameInstance::HandleWorldCleanup);

	// 만약 기본 슬롯 이름이 정해져 있다면 여기서 즉시 로드 시도
	// (예: SaveGameSlot = TEXT("SaveSlot_01");)
	if (!SaveGameSlot.IsEmpty())
	{
		LoadGame();
	}
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
