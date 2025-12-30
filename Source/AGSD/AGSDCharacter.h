// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "HoldingState.h"
#include "AGSDPlayerController.h"
#include "FadeWidget.h"
#include "HealthBar.h"
#include "HoldingWeapon.h"
#include "InputBufferEntry.h"
#include "PlayerStateWidget.h"
#include "SOVGameInstance.h"
<<<<<<< HEAD
=======
#include "Interface/PetConversationInterface.h"
>>>>>>> c424cfd17141ebd12aed7c0edfc6f53e79f50378
#include "AGSDCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class AACultivationPlot;

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

	AAGSDPlayerController* PC;
	
	UPROPERTY(VisibleAnywhere, Category="Input Buffer")
	TArray<FInputBufferEntry> InputBuffer;

	UPROPERTY(EditDefaultsOnly, Category="Input Buffer")
	float InputBufferDuration = 0.3f;
	
	UPROPERTY(EditDefaultsOnly, Category="Input Buffer")
	float InputDifference = 0.3f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UFadeWidget> WBP_FadeWidget;

	UPROPERTY()
	class UFadeWidget* FadeWidget;

private:
	UFUNCTION(BlueprintCallable)
	void HandleAttackInput(FName ActionName);
	
public:

	/** Constructor */
	AAGSDCharacter();	

	//G 키 입력과 바인딩될 함수
	void TryInteract();

	//AACultivationPlot에서 호출하여 상호작용 대상을 설정/초기화하는 함수
	FORCEINLINE void SetCurrentInteractableActor(AActor* NewActor) { CurrentInteractableActor = NewActor;}

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Item")
	FString getPlayerHoidingItemID();

	UFUNCTION(BlueprintCallable)
	//AACultivationPlot에서 호출하여 액터를 추가하는 함수
	void AddInteractableActor(AActor* NewActor);

	UFUNCTION(BlueprintCallable)
	//AACultivationPlot에서 호출하여 액터를 제거하는 함수
	void RemoveInteractableActor(AActor* ActorToRemove);
	//상호작용 가능 액터 개수
	FORCEINLINE int32 GetInteractableActorNum() const {return InteractableActorsInRange.Num();}
	
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputMappingContext* IMC_Farmer;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HoldingState")
	EHoldingState HoldingState = EHoldingState::EHS_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HoldingState")
	EHoldingWeapon HoldingWeapon = EHoldingWeapon::None;

	AActor* MinDistActor();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction")
	FString SubItemAmount();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	UHealthBar* getHealthBar();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction")
	void StrongAttack();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interaction")
	void Attack();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FORCEINLINE void SetCanOpenChest(bool boolean) { bCanOpenChest = boolean; };

	void SetHighLight(AActor* TargetActor, bool bActive);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanOpenChest = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool SkillMotion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Mining = false;

	void Die();
	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USOVGameInstance* GI;
	
	FORCEINLINE AAGSDPlayerController* getPlayerController() const {return PC;};
	FORCEINLINE USOVGameInstance* getPlayerGameInstance() const {return GI;};

	FORCEINLINE float getPlayerMaxhealth() const { return MaxHealth;}
	
protected:
	//현재 상호작용 가능한 액터 포인터 (AACultivationPlot 또는 ACrop 등)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	AActor* CurrentInteractableActor = nullptr;
	
	//현재 상호작용 가능한 모든 액터를 저장하는 TSet (중복 없이 빠른 추가/제거)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TSet<AActor*> InteractableActorsInRange = {}; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    bool CanInteract = false;
	
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerState")
	float MaxHealth = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerState")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
	float Damage = 100.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	bool bCanBeDamage = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerUI")
	class UHealthBar* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerUI")
	TSubclassOf<class UHealthBar> WBP_HealthBar;

	FORCEINLINE float getHealth() const {return Health;};
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	virtual void Jump() override;

	virtual void StopJumping() override;
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void ConsumeInputs(TArray<FInputBufferEntry>& Buffer, int32 Count);
	bool CheckCombo(
	const TArray<FInputBufferEntry>& Buffer, 
	FName Input1, 
	FName Input2, 
	float MaxTimeBetweenInputs);

	// AAGSDCharacter.h에 추가
	bool CheckSingleInput(const TArray<FInputBufferEntry>& Buffer, FName InputName);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PlayerUI")
	UPlayerStateWidget* PlayerStateWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UAnimMontage* DyingMontage;

	UFUNCTION()
	void WakeUp();
	
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
	
<<<<<<< HEAD
=======

	// 펫 관련 추가( 인터페이스를 추가하고 인터페이스 함수중 하나인 SetMyPet 구현 )
	//--

	/*
	#include "Interface/PetConversationInterface.h"
	#include "BaseFlyingPet.h"
	IPetConversationInterface::Execute_MasterToPetConversation(OtherActor, DialogueID); 캐릭터에서 실행하면 자동으로 대화 실행
	 */

	UPROPERTY( EditAnywhere, Category="Pet")
	bool bHasPet = false;
	
	UPROPERTY( BlueprintReadOnly)
	class ABaseFlyingPet* Pet;

	// 에디터의 Details 패널에서 어떤 펫 블루프린트를 쓸지 선택하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	TSubclassOf<class ABaseFlyingPet> DefaultPetClass;

	// 인터페이스 함수 구현
	virtual void SetMyPet_Implementation(AActor* NewPet) override;
	virtual void MasterToPetConversation_Implementation(FName DialogueID) override;

	// 레벨 이동 전 펫 파괴 및 이동 후 펫 스폰 함수
	UFUNCTION(BlueprintCallable)
	void DestroyPetBeforeTravel();
	UFUNCTION(BlueprintCallable)
	void SpawnMyPetAfterTravel();
	
	//--
	// 펫 관련 추가
	
public:
>>>>>>> c424cfd17141ebd12aed7c0edfc6f53e79f50378
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void WeaponAttack();

	UFUNCTION()
	void AddDamage(float addDamage);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void HealthRecovery(float amount);
};

