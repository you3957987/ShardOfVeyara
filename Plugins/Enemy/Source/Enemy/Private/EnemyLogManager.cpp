#include "EnemyLogManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

void UEnemyLogManager::EnemyLog(EEnemyLogType InEnemyLogType, FString Content)
{
	FString FileName = GetLogFileName(InEnemyLogType);
	
	// 저장 경로: Saved/Logs/Enemy/FileName.log
	FString SavePath = FPaths::ProjectSavedDir() + TEXT("Logs/Enemy/") + FileName + TEXT(".log");
	
	FString FinalLine = FString::Printf(TEXT("[%s] %s%s"), *FDateTime::Now().ToString(), *Content, LINE_TERMINATOR);

	FFileHelper::SaveStringToFile(FinalLine, *SavePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

FString UEnemyLogManager::GetLogFileName(EEnemyLogType InEnemyLogType)
{
	switch (InEnemyLogType)
	{	
		case EEnemyLogType::Melee: return TEXT("MeleeEnemy");
		case EEnemyLogType::Ranged: return TEXT("RangedEnemy");
		case EEnemyLogType::Exploder: return TEXT("ExploderEnemy");
		case EEnemyLogType::Transpar: return TEXT("TransparEnemy");
		case EEnemyLogType::Mimic: return TEXT("MimicEnemy");
		case EEnemyLogType::Slime: return TEXT("SlimeEnemy");
		case EEnemyLogType::Mage: return TEXT("MageEnemy");
		case EEnemyLogType::Guard: return TEXT("GuardEnemy");
		case EEnemyLogType::Passive: return TEXT("PassiveEnemy");
		case EEnemyLogType::Burrow: return TEXT("BurrowEnemy");
		case EEnemyLogType::Spawner: return TEXT("SpawnerEnemy");
		case EEnemyLogType::Revive: return TEXT("ReviveEnemy");
		
		case EEnemyLogType::SkeletonMage: return TEXT("SkeletonMageEnemy");
		case EEnemyLogType::BlackKnight: return TEXT("BlackKnightEnemy");
		case EEnemyLogType::Worm: return TEXT("WormEnemy");
		case EEnemyLogType::MagicSwordMan: return TEXT("MagicSwordManEnemy");
		default: return TEXT("UnknownEnemy");
	}
}
