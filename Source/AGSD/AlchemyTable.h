// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AGSDCharacter.h"
#include "AlchemyUI.h"
#include "Interaction.h"
#include "PotionDataTable.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "AlchemyTable.generated.h"

UCLASS()
class AGSD_API AAlchemyTable : public AActor, public IInteraction
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAlchemyTable();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneComponent;
	
	bool bCanUseAlchemyTable = true;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyPotColor(FLinearColor NewColor);

	UFUNCTION(BlueprintImplementableEvent)
	void LerpMixLiquidColor();

	UFUNCTION(BlueprintImplementableEvent)
	void TransmutationComplete();

	FText InteractActionText = FText::FromString(TEXT("양조하기"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float BlendTime = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UAlchemyUI> AlchemyWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	UAlchemyUI* AlchemyWidget;

	AAGSDCharacter* Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FString> InsertedItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CultivationPlot)
	class UDataTable* AlchemyDataTable;
	
public:
	void OnCameraBlendFinished();
	
	virtual void Interact_Implementation(AAGSDCharacter* player) override;
	virtual void ShowWidget_Implementation(ACharacter* player) override;
	virtual bool CanInteract_Implementation(AAGSDCharacter* player) override;

	//오버랩 시작 시 작동할 함수
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//오버랩 종료 시 작동할 함수
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UStaticMeshComponent* PotMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UNiagaraComponent* SplashVFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	class UNiagaraComponent* BubbleVFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FLinearColor BaseLiquidColor = FColor::FromHex("A5DDFFFF");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FLinearColor LiquidColor = BaseLiquidColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPotionData TargetRecipe;
	
	UFUNCTION(BlueprintCallable)
	void SplashPot(bool clear);

	UFUNCTION(BlueprintCallable)
	void SpawnPotion();
	
	UFUNCTION()
	void EndAlchemy();
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CultivationPlot, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMaterialInstanceDynamic* PotDynamicMaterial;

	TArray<FPotionData*> AlchemyRecipes;
};
