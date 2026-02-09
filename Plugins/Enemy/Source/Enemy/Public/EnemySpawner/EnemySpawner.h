#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

UCLASS()
class ENEMY_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	void PollInit(); // 틱에서 하는 초기화
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bDebugMode = false;

	// 캐릭터 초기화 여부를 나타내는 변수
	bool bTargetInitalize = false;
	// 타겟으로 삼을 캐릭터를 저장할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<ACharacter> TargetCharacter;
	
	// 루트 컴포넌트가 될 캡슐 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* RootCollisionSphere;
	
	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* SpawnerMesh;
	// 플레이어 감지 범위 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* PlayerDetectSphere;

	// 비긴 오버랩 함수
	UFUNCTION()
	void OnBeginOverlapPlayerDetectSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 플레이어 감지 종료 (스폰 중지) 
	UFUNCTION()
	void OnEndOverlapPlayerDetectSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 플레이어가 보이는지 여부 확인 함수
	bool CanSeePlayer() const;
	// 플레이어 감시 하는 딜레이
	float CurrentSpawnCooldown = 0.0f;
	
	// 몬스터가 스폰될 위치 씬 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* SpawnLocation;

	// 스폰할 적 클래스 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "자체설정")
	TArray<TSubclassOf<class ABaseEnemy>> SpawningEnemyClasses;
	
	// 스폰 딜레이
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float SpawnDelay = 7.0f;

	// 스폰 타이머 관리용 핸들 
	FTimerHandle SpawnTimerHandle;
	
	void SpawnEnemy(); // 적 스폰 함수

public:	
	AEnemySpawner();
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
