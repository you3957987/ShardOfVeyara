// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UCropData.h"
#include "Interaction.h"
#include "Crop.h"
#include "ACultivationPlot.generated.h"

UCLASS()
class AGSD_API AACultivationPlot : public AActor, public IInteraction
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this actor's properties
	AACultivationPlot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Farming")
	TSubclassOf<ACrop> CropClassToPlant;
	
	//작물 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Farming")
	UUCropData* CropData;

	bool bCanPlant = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	class ACrop* PlantedCrop = nullptr;

	void PlantCrop();

	UFUNCTION()
	void OnPlantedCropDestroyed(AActor* DestroyedActor);

	FText InteractActionText = FText::FromString(TEXT("작물심기"));

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//오버랩 시작 시 작동할 함수
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//오버랩 종료 시 작동할 함수
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Interact_Implementation(AAGSDCharacter* player) override;
	virtual void ShowWidget_Implementation(ACharacter* player) override;
private:
	//루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CultivationPlot, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* RootScene;
	//콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CultivationPlot, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;
};
