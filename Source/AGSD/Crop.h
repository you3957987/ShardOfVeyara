// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UCropData.h"
#include "Interaction.h"
#include "harvest.h"
#include "Crop.generated.h"

UCLASS()
class AGSD_API ACrop : public AActor, public IInteraction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrop();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//현재 단계의 작물 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CropMesh;
	UPROPERTY(EditAnywhere, Category = "Farming") // EditDefaultsOnly도 가능
	TSubclassOf<Aharvest> HarvestClass;
	//작물 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	UUCropData* CropData;
	//현재 단계
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 CurrentGrowStageIndex = 0;
	//다음 단계까지 남은 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 GrowthTimeCounter = 0;
	//다음 성장 날짜
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 ScheduledDay = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	bool FullyGrown = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 FinishGrowStageIndex = 0;

	void HarvestCrop();

	FText InteractActionText = FText::FromString(TEXT("수확하기"));

	void RegisterCropToManager(int32 GrowthTimeCounter);
public:	
	//작물이 경작지에 심길 때 실행할 함수
	void SetCropData(UUCropData* CData);
	//작물 매니저가 성장 날짜가 되었을 땨 호출하여 성장을 처리하는 함수
	void AdvanceGrowth();

	FORCEINLINE int32 GetGrowthTimeCounter() const { return GrowthTimeCounter; };
	FORCEINLINE int32 GetCurrentGrowStageIndex() const { return CurrentGrowStageIndex; };
	FORCEINLINE int32 GetScheduledDay() const { return ScheduledDay; };
	FORCEINLINE bool GetFullyGrown() const { return FullyGrown; };

	//오버랩 시작 시 작동할 함수
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//오버랩 종료 시 작동할 함수
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Interact_Implementation(AAGSDCharacter* player) override;
	virtual void ShowWidget_Implementation(ACharacter* player) override;

private:
	//작물 메시 정보 업데이트
	void MeshUpdate(int32 CurrentGrowStageIndex);

	//콜리전 박스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CultivationPlot, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CollisionBox;
};
