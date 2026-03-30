#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealActor.generated.h"

UCLASS()
class ENEMY_API AHealActor : public AActor
{
	GENERATED_BODY()


protected:
	virtual void BeginPlay() override;

public:	
	AHealActor();
	virtual void Tick(float DeltaTime) override;

	// 플레이어 충돌 감지 스피어 콜리전 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "자체설정")
	class USphereComponent* CollisionSphere;
	
	// 비긴 오버랩 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
