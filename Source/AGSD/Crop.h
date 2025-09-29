// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UCropData.h"
#include "Crop.generated.h"

UCLASS()
class AGSD_API ACrop : public AActor
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
	//작물 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	UUCropData* CropData;
	//현재 단계
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 CurrentGrowStageIndex = 0;
	//다음 단계까지 남은 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 GrowthTimeCounter = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	int32 ScheduledDay = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
	ACropManager* Manager;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//작물이 경작지에 심길 때 실행할 함수
	void SetCropData(UUCropData* CropData);
	//작물 매니저가 성장 날짜가 되었을 땨 호출하여 성장을 처리하는 함수
	void AdvanceGrowth();

	FORCEINLINE int32 GetGrowthTimeCounter() const { return GrowthTimeCounter; };
private:
	//작물 메시 정보 업데이트
	void MeshUpdate();
};
