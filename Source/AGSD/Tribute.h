// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction.h"
#include "TributeUI.h"
#include "GameFramework/Actor.h"
#include "Tribute.generated.h"

UCLASS()
class AGSD_API ATribute : public AActor, public IInteraction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATribute();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tribute")
	TObjectPtr<class UWidgetComponent> TributeUI;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tribute")
	class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tribute")
	class UTributeUI* TributeUIInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	UDataTable* TributeDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribute")
	int32 TributeLevel = 0;

	FText InteractActionText = FText::FromString(TEXT("봉헌하기"));

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tribute")
	TMap<FString, int32> CurrentLevelTributeItems = {};

	void SetNextTributeUI();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(AAGSDCharacter* player) override;
	virtual void ShowWidget_Implementation(ACharacter* player) override;
	virtual bool CanInteract_Implementation(AAGSDCharacter* player) override;

	//오버랩 시작 시 작동할 함수
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//오버랩 종료 시 작동할 함수
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tribute")
	void PlayFireNiagara();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Tribute")
	void PlayFireExplosionNiagara();
};
