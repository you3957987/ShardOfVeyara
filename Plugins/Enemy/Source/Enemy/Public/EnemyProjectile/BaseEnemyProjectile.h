#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEnemyProjectile.generated.h"

UCLASS()
class ENEMY_API ABaseEnemyProjectile : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;


	// 발사체 속도
	UPROPERTY(EditAnywhere, Category="자체설정")
	float ProjectileSpeed = 3000.f; // 초기 속도
	UPROPERTY(EditAnywhere, Category="자체설정")
	float Damage = 20.f; // 데미지
	
	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComp;
	// 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;
	// 투사체 이동 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovement;

public:
	ABaseEnemyProjectile();
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }


	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
