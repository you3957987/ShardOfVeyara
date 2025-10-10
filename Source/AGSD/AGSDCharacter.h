// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AGSDCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AAGSDCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* Interaction;

public:

	/** Constructor */
	AAGSDCharacter();	

	//G 키 입력과 바인딩될 함수
	void TryInteract();

	//AACultivationPlot에서 호출하여 상호작용 대상을 설정/초기화하는 함수
	FORCEINLINE void SetCurrentInteractableActor(AActor* NewActor) { CurrentInteractableActor = NewActor;};

	//AACultivationPlot에서 호출하여 액터를 추가하는 함수
	void AddInteractableActor(AActor* NewActor);
	//AACultivationPlot에서 호출하여 액터를 제거하는 함수
	void RemoveInteractableActor(AActor* ActorToRemove);
	//상호작용 가능 액터 개수
	FORCEINLINE int32 GetInteractableActorNum() const {return InteractableActorsInRange.Num();}
	
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputMappingContext* IMC_Farmer;

	virtual void Tick(float DeltaSeconds) override;
protected:

	//현재 상호작용 가능한 액터 포인터 (AACultivationPlot 또는 ACrop 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	AActor* CurrentInteractableActor = nullptr;
	
	//현재 상호작용 가능한 모든 액터를 저장하는 TSet (중복 없이 빠른 추가/제거)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSet<AActor*> InteractableActorsInRange; 
	
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

