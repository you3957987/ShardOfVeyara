#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageZoneOnGroundProjectile.generated.h"

UCLASS()
class ENEMY_API ADamageZoneOnGroundProjectile : public AActor
{
	GENERATED_BODY()


protected:
	virtual void BeginPlay() override;

public:	
	ADamageZoneOnGroundProjectile();
	virtual void Tick(float DeltaTime) override;

	// 장판 유지 시간
	UPROPERTY(EditAnywhere, Category="자체설정")
	float ZoneDuration = 5.f;
	
	
};
