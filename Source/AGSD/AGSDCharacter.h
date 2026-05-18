// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "HoldingState.h"
#include "AGSDPlayerController.h"
#include "AnimNotifyState_WeaponAttack.h"
#include "FadeWidget.h"
#include "HealthBar.h"
#include "HoldingWeapon.h"
#include "InputBufferEntry.h"
#include "PlayerStateWidget.h"
#include "SOVGameInstance.h"
#include "Interface/PetConversationInterface.h"
#include "Interface/PlayerDeadInterface.h"
#include "Interface/ItemDropInterface.h"
#include "Interface/InteractionInterface.h"
#include "SpearComboData.h"
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
class AGSD_API AAGSDCharacter : public ACharacter, public IPetConversationInterface, public IPlayerDeadInterface
	, public IItemDropInterface, public IInteractionInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	bool bIsBlocking = false;
	
	bool bIsJustGuardWindow = false;

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

	// 가드 액션
	UPROPERTY(EditAnywhere, Category="Input")
	const UInputAction* GuardAction;

	// 락온 액션
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputAction* LockOnAction;
	
	// 락온 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|LockOn")
	AActor* LockedTarget = nullptr;

	// 락온 탐색 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|LockOn")
	float LockOnRadius = 1500.0f;

	// 락온 관련 함수
	void ToggleLockOn();
	AActor* FindNearestLockOnTarget();
	void SetLockOnMarkerState(AActor* TargetActor, bool bActive);
	
	AAGSDPlayerController* PC;
	
	UPROPERTY(VisibleAnywhere, Category="Input Buffer")
	TArray<FInputBufferEntry> InputBuffer;

	UPROPERTY(EditDefaultsOnly, Category="Input Buffer")
	float InputBufferDuration = 0.3f;
	
	UPROPERTY(EditDefaultsOnly, Category="Input Buffer")
	float InputDifference = 0.3f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input Buffer")
	float AttackBufferDuration = 0.2f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UFadeWidget> WBP_FadeWidget;

	UPROPERTY(BlueprintReadOnly)
	class UFadeWidget* FadeWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerState")
	int Coin;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="PlayerState")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="PlayerState")
	bool bIsRecovering = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="PlayerState")
	bool bCanCombo = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="PlayerState")
	bool bHasBufferedInput = false;

	// 선입력된 공격의 발생 시점 (유효 시간 체크용)
	float BufferedInputTime = 0.0f;

	// USTRUCT 포인터는 UPROPERTY로 관리할 수 없으므로 매크로 제거
	FSpearComboData* CurrentComboData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="PlayerState")
	int32 CurrentStageIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerState")
	UDataTable* SpearComboDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PlayerState")
	class UAnimMontage* BlockStartMontage;

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
	FORCEINLINE float getDamage() const {return Damage;};

	UPROPERTY(EditAnywhere, Category="Input")
	class UInputMappingContext* IMC_Farmer;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HoldingState")
	EHoldingState HoldingState = EHoldingState::EHS_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HoldingState")
	EHoldingWeapon HoldingWeapon = EHoldingWeapon::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="HoldingState")
	AActor* HoldingActor;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Weeding = false;

	void Die();
	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USOVGameInstance* GI;
	
	FORCEINLINE AAGSDPlayerController* getPlayerController() const {return PC;};
	FORCEINLINE USOVGameInstance* getPlayerGameInstance() const {return GI;};

	FORCEINLINE float getPlayerMaxhealth() const { return MaxHealth;}

	FORCEINLINE void SetCanCombo(bool b) {bCanCombo = b;}
	
	FORCEINLINE bool HasBufferedInput() {return bHasBufferedInput;}
	
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
	float Damage = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerState")
	bool bCanBeDamage = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerUI")
	class UHealthBar* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerUI")
	TSubclassOf<class UHealthBar> WBP_HealthBar;

	FORCEINLINE float getHealth() const {return Health;};
	
	// 선입력 및 방향 판정을 위한 원시 입력 벡터 (이동 제한 상태에서도 업데이트됨)
	FVector LastRawInputVector = FVector::ZeroVector;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	void StopMove();
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* Running;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* Jumping;

	UFUNCTION(BlueprintCallable)
	void playFadeWidget(float startOpacity, float endOpacity);

public:
	// Enemy 로그 처리 위한 겟 함수
	FORCEINLINE float GetFarmerHealthToEnemy() const { return Health; }

	void OnRecoveryFinished(UAnimMontage* Montage, bool bInterrupted);
	void ResetAttackState();
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
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "PlayerState")
	void TeleportToBed();

	UPROPERTY( EditAnywhere, Category="Pet")
	bool bHasPet = false;
	
	UPROPERTY( BlueprintReadOnly)
	class ABaseFlyingPet* Pet;

	// 에디터의 Details 패널에서 어떤 펫 블루프린트를 쓸지 선택하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pet")
	TSubclassOf<class ABaseFlyingPet> DefaultPetClass;

	// 인터페이스 함수 구현
	virtual void SetMyPet_Implementation(AActor* NewPet) override;
	virtual void MasterToPetBigConversation_Implementation(FName DialogueID) override;

	// 레벨 이동 전 펫 파괴 및 이동 후 펫 스폰 함수
	UFUNCTION(BlueprintCallable)
	void DestroyPetBeforeTravel();
	UFUNCTION(BlueprintCallable)
	void SpawnMyPetAfterTravel();
	
	// 로그 관련 인터페이스 함수 구현
	UPROPERTY()
	FOnPlayerDeadSignature OnPlayerDead;
	virtual FOnPlayerDeadSignature& ReturnOnPlayerDeadDelegate() override { return OnPlayerDead; }
	
	// 락온 상태 변경 인터페이스 구현
	UPROPERTY()
	FOnLockOnStateChanged OnLockOnStateChanged;
	virtual FOnLockOnStateChanged& GetLockOnStateChangedDelegate() override { return OnLockOnStateChanged; }
	
	virtual void StopAnimMontage(UAnimMontage* AnimMontage = nullptr) override;
	
	UFUNCTION()
	void HandleLockOn(bool bLockOn);
	
	// 적이 드롭할 아이템 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemDrop")
	class UDataTable* EnemyDropDataTable;
	virtual void HandleEnemyDeadAndDropItem_Implementation( AActor* DeadActor ) override;
	
	FORCEINLINE AActor* getCurrentInteractableActor() const { return CurrentInteractableActor; }
	//--
	// 펫 관련 추가
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	float WeaponDamage();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void WeaponAttack();

	UFUNCTION()
	void AddDamage(float addDamage);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void HealthRecovery(float amount);

	ESpearAttackDirection GetAttackDirection();

	UFUNCTION(BlueprintCallable)
	void ProcessAttackInput();
	
	void StartNewCombo();
	
	FSpearComboData* GetComboDataByDirection(ESpearAttackDirection Direction);
	
	void ExecuteNextStage();
	
	void PlayStage(int32 Index);
	
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void StartRecovery(UAnimMontage* RecoveryMontage);

	FTimerHandle JustGuardTimerHandle;
	void EndJustGuardWindow();

	void StartBlock();
	void StopBlock();
	void OnHitReceived();

	/** 저스트 가드 성공 효과 처리 */
	void HandleJustGuardSuccess();
	void ResetCombo();
	/** 성공 시 재생할 이펙트 */
	UPROPERTY(EditAnywhere, Category = "Combat|Effects")
	UParticleSystem* JustGuardParticle;
	/** 성공 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, Category = "Combat|Effects")
	USoundBase* JustGuardSound;
	
};

