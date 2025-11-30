// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction.h"
#include "GameFramework/Actor.h"
#include "AGSDCharacter.h"
#include "PickUpItem.generated.h"
/**
 * 
 */
UCLASS()
class AGSD_API APickUpItem : public AActor, public IInteraction
{
	GENERATED_BODY()
	
public:
	APickUpItem();

	virtual void BeginPlay() override;;

	//오버랩 시작 시 작동할 함수
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//오버랩 종료 시 작동할 함수
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	bool Holding = false;
	
	FText InteractActionText = FText::FromString(TEXT("수집하기"));

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
	void PickUpInteract(AAGSDCharacter* player);
	
public:
	virtual void Interact_Implementation(AAGSDCharacter* player) override;
	virtual void ShowWidget_Implementation(ACharacter* player) override;
	virtual bool CanInteract_Implementation(AAGSDCharacter* player) override;

private:
	//콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CultivationPlot, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CollisionBox;
};
