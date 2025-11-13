// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction.h"
#include "GameFramework/Actor.h"
#include "harvest.generated.h"

class UUCropData;

UCLASS()
class AGSD_API Aharvest : public AActor, public IInteraction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	Aharvest();

	//오버랩 시작 시 작동할 함수
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//오버랩 종료 시 작동할 함수
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FORCEINLINE UStaticMeshComponent* GetMeshComponent() const { return HarvestMesh; };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//현재 단계의 작물 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HarvestMesh;

	FText InteractActionText = FText::FromString(TEXT("수집하기"));

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
	void HarvestInteract(AAGSDCharacter* player);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
	AAGSDCharacter* InteractingPlayer = nullptr;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation() override;
	virtual void ShowWidget_Implementation(ACharacter* player) override;
	
private:
	//콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CultivationPlot, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CollisionBox;
};
