#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpaner.generated.h"

UCLASS()
class ENEMY_API AEnemySpaner : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	// 루트 컴포넌트가 될 스피어
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* RootCollisionSphere;
	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* SpawnerMesh;

	// 몬스터가 스폰될 위치 씬 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* SpawnLocation;

public:	
	AEnemySpaner();
	virtual void Tick(float DeltaTime) override;

};
