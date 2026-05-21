// Copyright Epic Games, Inc. All Rights Reserved.

#include "AGSDCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interaction.h"
#include "Struct_ItemData.h"
#include "UsableItem.h"
#include "AGSD.h"
#include "FadeWidget.h"
#include "HeartProgressBar.h"
#include "PlayerStateWidget.h"
#include "MotionWarpingComponent.h"
#include "SOVGameInstance.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "BaseFlyingPet.h"
#include "SpearComboData.h"
#include "Components/AudioComponent.h"
#include "Interface/ItemDropInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#if WITH_EDITOR
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#endif

AAGSDCharacter::AAGSDCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//오디오 컴포넌트
	Running = CreateDefaultSubobject<UAudioComponent>(TEXT("RunningAudio"));
	Running->SetupAttachment(RootComponent);
	Running->bAutoActivate = false;

	Jumping = CreateDefaultSubobject<UAudioComponent>(TEXT("JumpingAudio"));
	Jumping->SetupAttachment(RootComponent);
	Jumping->bAutoActivate = false;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void AAGSDCharacter::HandleAttackInput(FName ActionName)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	int32 CurrentFrame = GFrameCounter;

	FInputBufferEntry NewEntry(ActionName, CurrentTime, CurrentFrame);

	InputBuffer.Add(NewEntry);
}

void AAGSDCharacter::TryInteract()
{
	// 1. 플레이어 근처에 상호작용 가능한 오브젝트가 있으면 최우선으로 상호작용
	if (CanInteract && IsValid(CurrentInteractableActor) && CurrentInteractableActor->Implements<UInteraction>())
	{
		IInteraction::Execute_Interact(CurrentInteractableActor, this);
		return;
	}

	// 2. 상호작용 가능한 오브젝트가 없으면, 들고 있는 아이템 데이터(HoldingItemData)의 클래스로부터 CDO를 가져와 사용 시도
	if (HoldingItemData.ItemBPClass)
	{
		// 클래스 디폴트 오브젝트(CDO)를 가져와 전용 사용 인터페이스(IUsableItem) 실행 (스폰 없음)
		AActor* DefaultActor = Cast<AActor>(HoldingItemData.ItemBPClass->GetDefaultObject());
		if (DefaultActor)
		{
			if (DefaultActor->Implements<UUsableItem>())
			{
				IUsableItem::Execute_UseItem(DefaultActor, this);
			}
		}
	}
}

void AAGSDCharacter::AddInteractableActor(AActor* NewActor)
{
	if (NewActor) InteractableActorsInRange.Add(NewActor);
	CanInteract = true;
}

void AAGSDCharacter::RemoveInteractableActor(AActor* ActorToRemove)
{
	if (ActorToRemove) InteractableActorsInRange.Remove(ActorToRemove);
	if(InteractableActorsInRange.Num() <= 0) CanInteract = false;
}

void AAGSDCharacter::ConsumeInputs(TArray<FInputBufferEntry>& Buffer, int32 Count)
{
	// 버퍼가 충분한 입력을 가지고 있는지 확인
	if (Buffer.Num() >= Count)
	{
		// Count만큼 가장 최근 입력(배열의 끝)부터 제거
		for (int32 i = 0; i < Count; ++i)
		{
			Buffer.RemoveAt(Buffer.Num() - 1);
		}
	}
}

bool AAGSDCharacter::CheckCombo(
	const TArray<FInputBufferEntry>& Buffer, 
	FName Input1, 
	FName Input2, 
	float MaxTimeBetweenInputs)
{
	// 1. 최소 길이 확인
	if (Buffer.Num() < 2)
	{
		return false;
	}

	// 2. 최근 입력 두 개 가져오기 (직접 접근)
	const FInputBufferEntry& Entry2 = Buffer[Buffer.Num() - 1]; // Input2 (가장 최근)
	const FInputBufferEntry& Entry1 = Buffer[Buffer.Num() - 2]; // Input1 (바로 이전)
    
	// 3. 순서 확인
	bool bSequenceMatch = (Entry1.InputName == Input1) && (Entry2.InputName == Input2);

	if (!bSequenceMatch)
	{
		return false;
	}

	// 4. 시간 간격 확인
	float TimeDifference = Entry2.TimeStamp - Entry1.TimeStamp;

	// 두 입력 사이의 시간 간격이 허용 시간 내에 있는지 확인
	if (TimeDifference <= MaxTimeBetweenInputs)
	{
		return true; // 커맨드 인식 성공!
	}

	return false;
}

bool AAGSDCharacter::CheckSingleInput(const TArray<FInputBufferEntry>& Buffer, FName InputName)
{
	// 1. 최소 길이 확인
	if (Buffer.Num() < 1)
	{
		return false;
	}

	// 2. 가장 최근 입력의 이름이 우리가 찾는 InputName과 일치하는지 확인
	// (시간 유효성은 이미 Tick 시작 부분의 정리 로직에서 검증되었으므로 시간 비교는 필요 없음)
	const FInputBufferEntry& LastEntry = Buffer[Buffer.Num() - 1];

	return LastEntry.InputName == InputName;
}

void AAGSDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// --- [락온 카메라 회전 및 유지 상태 처리] ---
	if (LockedTarget)
	{
		bool bShouldRelease = false;

		// 1. 적이 파괴되었는지 검사 (기존 예외 처리)
		if (!IsValid(LockedTarget)) 
		{
			bShouldRelease = true;
		}
		else
		{
			// 2. 락온 유지 한계 거리(1800.0f) 체크
			float Distance = FVector::Dist(GetActorLocation(), LockedTarget->GetActorLocation());
			if (Distance > 1800.0f)
			{
				bShouldRelease = true;
			}
		}

		if (bShouldRelease)
		{
			ToggleLockOn();
		}
		else
		{
			// 3. 장애물 시야 차단 체크 (Line of Sight - Visibility 채널)
			FVector TraceStart = GetFollowCamera()->GetComponentLocation();
			FVector TraceEnd = LockedTarget->GetActorLocation();
			
			FCollisionQueryParams TraceParams;
			TraceParams.AddIgnoredActor(this);
			TraceParams.AddIgnoredActor(LockedTarget);

			FHitResult HitResult;
			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

			if (bHit)
			{
				// 장애물에 가려진 상태
				if (!bIsLineOfSightBlocked)
				{
					bIsLineOfSightBlocked = true;
					// 1.2초 후 OnLineOfSightTimeout 실행
					GetWorldTimerManager().SetTimer(LineOfSightTimerHandle, this, &AAGSDCharacter::OnLineOfSightTimeout, 1.2f, false);
				}
			}
			else
			{
				// 시야가 확보된 상태
				if (bIsLineOfSightBlocked)
				{
					bIsLineOfSightBlocked = false;
					GetWorldTimerManager().ClearTimer(LineOfSightTimerHandle);
				}
			}

			// 4. 카메라 회전 보간 처리
			FVector CameraLocation = GetFollowCamera()->GetComponentLocation();
			FVector TargetLocation = LockedTarget->GetActorLocation(); 
			
			// 몬스터의 발끝을 보지 않도록 Z축 보정
			TargetLocation.Z -= 50.f; 

			FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation);
			FRotator CurrentRotation = GetController()->GetControlRotation();
			
			TargetRotation.Pitch = CurrentRotation.Pitch;
			TargetRotation.Roll = 0.0f;

			FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, 8.0f);
			GetController()->SetControlRotation(SmoothedRotation);
		}
	}
	// ---------------------------------------------

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 1. 유효 시간이 지난 입력 제거 (버퍼 정리)
	// 오래된 입력은 앞에서부터 제거합니다. (FIFO: First In, First Out)
	for (int32 i = InputBuffer.Num() - 1; i >= 0; --i)
	{
		if (CurrentTime - InputBuffer[i].TimeStamp > InputBufferDuration)
		{
			InputBuffer.RemoveAt(i);
		}
	}

	if (!SkillMotion)
	{
		// 2. 가장 복잡한 커맨드 (StrongAttack 콤보) 먼저 확인
		// CheckCombo는 Input1="Attack", Input2="Attack"으로 설정해야 합니다. (Attack 2번 연속 입력 가정)
		if (CheckCombo(InputBuffer, FName("Forward"), FName("Attack"), InputDifference)) 
		{
			// 커맨드 인식 성공 시
			StrongAttack();
            
			// ⭐ 수정 1: 사용된 입력 2개를 버퍼에서 제거 ⭐
			ConsumeInputs(InputBuffer, 2); 
		}
        
		// 3. StrongAttack 콤보가 인식되지 않았을 경우, 단일 Attack 입력이 남아있는지 확인
		// 이전 입력 (Attack 2번째)이 너무 늦었거나, Attack 1개만 들어왔을 경우
		else if (CheckSingleInput(InputBuffer, FName("Attack"))) // 버퍼에 입력이 남아있는 경우
		{
			Attack();  
                
				// ⭐ 수정 3: 사용된 입력 1개를 버퍼에서 제거 ⭐
			ConsumeInputs(InputBuffer, 1);
		}
	}

	AActor* MinDistanceActor = MinDistActor();

	if (CurrentInteractableActor == MinDistanceActor) return;
	
	//CurrentInteractableActor 메시의 커스텀 텝스 패스 랜더 비활성화
	SetHighLight(CurrentInteractableActor, false);
	CurrentInteractableActor = MinDistanceActor;
	if (CurrentInteractableActor != nullptr)
	{
		IInteraction::Execute_ShowWidget(CurrentInteractableActor, this);
		//CurrentInteractableActor 메시의 커스텀 텝스 패스 랜더 활성화
		SetHighLight(CurrentInteractableActor, true);
	}
	else
	{
		if(PC) PC->HideInteractionWidget();
	}
}

void AAGSDCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnLockOnStateChanged.AddDynamic(this, &AAGSDCharacter::HandleLockOn);

	PC = Cast<AAGSDPlayerController>(GetController());
	
	if (PC)
	{
		// 카메라 거리에 따른 오디오 감쇠 문제를 해결하기 위해 오디오 리스너를 캐릭터(RootComponent)로 덮어씁니다.
		PC->SetAudioListenerOverride(GetRootComponent(), FVector::ZeroVector, FRotator::ZeroRotator);
	}

	GI = Cast<USOVGameInstance>(GetGameInstance());

	if (GI)
	{
		bHasPet = GI->bHasPet;
		Coin = GI->Coin;
		Damage = GI->Damage;
	}
	SpawnMyPetAfterTravel(); // 펫 있으면 오픈 레벨 이후 펫 스폰
	
	HealthBar = getHealthBar();
	if (HealthBar)
	{
		if (GI->MaxPlayerHealth != 0.f)
		{
			Health = GI->PlayerHealth;
			MaxHealth = GI->MaxPlayerHealth;
		}
		HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
	}
	playFadeWidget(1.0f, 0.0f);
	
	
	MouseSensitivity = GI->MouseSensitivity;
}

void AAGSDCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyPetBeforeTravel();
	Super::EndPlay(EndPlayReason);

	GI->Damage = Damage;
	GI->Coin = Coin;
	GI->PlayerHealth = Health;
	GI->MaxPlayerHealth = MaxHealth;
	GI->bHasPet = bHasPet;
}

AActor* AAGSDCharacter::MinDistActor()
{
	float MinDistance = FLT_MAX;

	const FVector PlayerLocation = GetActorLocation();

	AActor* MinDistanceActor = nullptr;
	
	for (AActor* CurrentActor : InteractableActorsInRange)
	{
		if (!CurrentActor) continue;

		if (!IInteraction::Execute_CanInteract(CurrentActor, this)) continue;
		
		const float DistanceSq = FVector::DistSquared(PlayerLocation, CurrentActor->GetActorLocation());

		if (DistanceSq < MinDistance)
		{
			MinDistance = DistanceSq;
			MinDistanceActor = CurrentActor;
		}
	}

	return MinDistanceActor;
}

void AAGSDCharacter::SetHighLight(AActor* TargetActor, bool bActive)
{
	if (TargetActor)
	{
		TArray<UPrimitiveComponent*> Comps;
		TargetActor->GetComponents(Comps);
		for (UPrimitiveComponent* Comp : Comps)
		{
			Comp->SetRenderCustomDepth(bActive);
		}
	}
}

void AAGSDCharacter::WakeUp()
{
	FadeWidget->OnFadeFinished.RemoveDynamic(this, &AAGSDCharacter::WakeUp);
	HealthRecovery(10.f);
	TeleportToBed();
}

void AAGSDCharacter::Die()
{
	DisableInput(PC);
	
	// 1. 애니메이션 인스턴스 가져오기
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 2. 안전성 검사 (AnimInstance와 Montage가 존재하는지)
	if (AnimInstance && DyingMontage)
	{
		// 3. 몽타주 재생
		// PlaySlotAnimationAsDynamicMontage 등 여러 방법이 있지만, 일반 몽타주는 아래 함수가 표준입니다.
		AnimInstance->Montage_Play(DyingMontage);
		
		FTimerHandle TimerHandle;// 2. 대기 시간 (초 단위)
		GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			playFadeWidget(0.0f, 1.0f);
			if (FadeWidget)
			{
				FadeWidget->OnFadeFinished.RemoveDynamic(this, &AAGSDCharacter::WakeUp);
				FadeWidget->OnFadeFinished.AddDynamic(this, &AAGSDCharacter::WakeUp);
			}
		}),
		1.0f, false
		);
	}
	
	if ( OnPlayerDead.IsBound() ) OnPlayerDead.Broadcast(); // 죽었다고 알리기
}

float AAGSDCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                 class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bCanBeDamage) return 0.f;
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (bIsJustGuardWindow) DamageToApply = 0.f;
	else if (bIsBlocking) DamageToApply = DamageToApply / 2.f;

	UE_LOG(LogTemp, Warning, TEXT("Player Take Damage : %f"), DamageToApply);

	if ( DamageToApply > 0.f )
	{
		Health = FMath::Clamp(Health - DamageToApply, 0, MaxHealth);
		if (HealthBar) HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
		if ( Health <= 0.f )
		{
			bCanBeDamage = false;
			Die();
		}
	}
	
	return DamageToApply;
}

void AAGSDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		if (EnhancedInputComponent != nullptr)
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
			{
				UEnhancedInputLocalPlayerSubsystem* EnhancedSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
				if (EnhancedSubsystem)
				{
					EnhancedSubsystem->AddMappingContext(IMC_Farmer, 1);
				}
			}
		}
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAGSDCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAGSDCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AAGSDCharacter::StopMove);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::Look);

		//G 키를 누를 때 TryInteract 함수를 호출하도록 바인딩
		EnhancedInputComponent->BindAction(Interaction, ETriggerEvent::Triggered, this, &AAGSDCharacter::TryInteract);

		//
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &AAGSDCharacter::StartBlock);
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &AAGSDCharacter::StopBlock);

		// 락온 기능 바인딩
		if (LockOnAction)
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AAGSDCharacter::ToggleLockOn);
		}

		if (SwitchTargetLeftAction)
		{
			EnhancedInputComponent->BindAction(SwitchTargetLeftAction, ETriggerEvent::Started, this, &AAGSDCharacter::SwitchTargetLeft);
		}

		if (SwitchTargetRightAction)
		{
			EnhancedInputComponent->BindAction(SwitchTargetRightAction, ETriggerEvent::Started, this, &AAGSDCharacter::SwitchTargetRight);
		}

		// 튜토리얼 스킵 기능 바인딩
		if (SkipTutorialAction)
		{
			EnhancedInputComponent->BindAction(SkipTutorialAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::SkipTutorialPressed);
		}
	}
	else
	{
		UE_LOG(LogAGSD, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAGSDCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Running)
	{
		// 입력이 있고(움직임), 마이닝 중이 아닐 때
		if (MovementVector.SizeSquared() > 0.f && !Mining
			&& !GetCharacterMovement()->IsFalling() && GetCharacterMovement()->Velocity.SizeSquared() > 9.f)
		{
			if (!Running->IsPlaying())
			{
				Running->Play();
			}
		}
		else
		{
			// 입력이 없거나 마이닝 중이면 사운드 정지
			if (Running->IsPlaying())
			{
				Running->Stop();
			}
		}
	}
	
	// 전진 키를 누르고 있을 때 콤보 입력
	if (MovementVector.Y > 0.0f) HandleAttackInput(FName("Forward"));

	// 선입력 판정을 위한 원시 입력 벡터 저장 (Mining 중에도 업데이트하여 애니메이션 중 방향 감지 가능)
	if (GetController())
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		LastRawInputVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * MovementVector.Y + 
							 FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * MovementVector.X;
	}

	// 움직이면 안될 때
	if (Mining)	return;
	
	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAGSDCharacter::StopMove()
{
	LastRawInputVector = FVector::ZeroVector;

	if (Running && Running->IsPlaying())
	{
		Running->Stop();
	}
}

void AAGSDCharacter::Jump()
{
	if (Mining) return;
 	if (Jumping && !GetCharacterMovement()->IsFalling())
	{
		Jumping->Play();
	}
	Super::Jump();
}

void AAGSDCharacter::StopJumping()
{
	if (Mining) return;
	Super::StopJumping();
}

void AAGSDCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AAGSDCharacter::playFadeWidget(float startOpacity, float endOpacity, float fadeSpeed)
{
	if (WBP_FadeWidget) 
	{
		if (!FadeWidget) FadeWidget = CreateWidget<UFadeWidget>(PC, WBP_FadeWidget);
                
		if (FadeWidget)
		{
			FadeWidget->SetFadeSpeed(fadeSpeed);
			FadeWidget->SetRenderOpacity(startOpacity);
			FadeWidget->SetTargetOpacity(endOpacity);
			if (!FadeWidget->IsInViewport()) FadeWidget->AddToViewport(100);
		}
	}	
}

ESpearAttackDirection AAGSDCharacter::GetAttackDirection()
{
	// [개선] GetLastMovementInputVector() 대신 직접 저장한 LastRawInputVector 사용
	// 애니메이션 도중에는 캐릭터 이동이 멈춰있어 GetLastMovementInputVector()가 0이 되기 때문
	FVector InputMoveVector = LastRawInputVector;
	if (InputMoveVector.IsNearlyZero()) return ESpearAttackDirection::Neutral;

	FVector Forward = GetActorForwardVector();
	InputMoveVector.Normalize();

	// 내적 연산 (Dot Product)
	float Dot = FVector::DotProduct(Forward, InputMoveVector);

	if (Dot > 0.5f) return ESpearAttackDirection::Forward;
	if (Dot < -0.5f) return ESpearAttackDirection::Backward;
    
	return ESpearAttackDirection::Neutral;
}

void AAGSDCharacter::ProcessAttackInput()
{
	// 공중 상태 체크
	if (GetCharacterMovement() && GetCharacterMovement()->IsFalling()) return;
	
	// 공격 중이거나 복귀 중인 경우
	if (bIsAttacking || bIsRecovering) 
	{
		// 1. 공격 중 콤보 가능 구간인 경우 -> 다음 단계 연계
		if (bCanCombo)
		{
			ExecuteNextStage();
		}
		// 2. 복귀 중인 경우 -> 현재 복귀를 중단하고 새로운 콤보 시작 (Cancel Recovery)
		else if (bIsRecovering)
		{
			StartNewCombo();
		}
		// 3. 그 외 공격 중인 경우 -> 입력 버퍼링
		else
		{
			bHasBufferedInput = true;
			BufferedInputTime = GetWorld()->GetTimeSeconds(); // 선입력 시점 저장
		}
		return;
	}

	// 완전히 Idle 상태인 경우 새로운 콤보 시작
	StartNewCombo();
}

void AAGSDCharacter::StartNewCombo()
{
	if (Mining) return;
	ESpearAttackDirection CurrentDir = GetAttackDirection();
	Mining = true;
	
	// 방향에 맞는 콤보 데이터를 테이블에서 한 번만 가져옴
	CurrentComboData = GetComboDataByDirection(CurrentDir);

	if (CurrentComboData && CurrentComboData->Stages.Num() > 0)
	{
		CurrentStageIndex = 0;
		PlayStage(0);
	}
}

FSpearComboData* AAGSDCharacter::GetComboDataByDirection(ESpearAttackDirection Direction)
{
	if (!SpearComboDataTable) return nullptr;

	// 데이터 테이블의 모든 행을 순회하며 방향 조건이 맞는 데이터를 검색
	static const FString ContextString(TEXT("Spear Attack Context"));
	TArray<FSpearComboData*> AllRows;
	SpearComboDataTable->GetAllRows<FSpearComboData>(ContextString, AllRows);

	for (FSpearComboData* ComboData : AllRows)
	{
		if (ComboData && ComboData->DirectionRequirement == Direction)
		{
			return ComboData;
		}
	}

	return nullptr;
}

void AAGSDCharacter::ExecuteNextStage()
{
	int32 NextIndex = CurrentStageIndex + 1;

	// 다음 스테이지가 존재할 경우에만 실행
	if (CurrentComboData && CurrentComboData->Stages.IsValidIndex(NextIndex))
	{
		CurrentStageIndex = NextIndex;
		PlayStage(NextIndex);
	}
}

void AAGSDCharacter::PlayStage(int32 Index)
{
	if (!CurrentComboData) return;

	FSpearStageData& Stage = CurrentComboData->Stages[Index];
	if (Stage.AttackMontage)
	{
		UpdateMotionWarpTarget();

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		float Duration = PlayAnimMontage(Stage.AttackMontage);
        
		if (Duration > 0.f)
		{
			bIsAttacking = true;
			bIsRecovering = false; // 새로운 공격 시작 시 복귀 상태 해제
			bCanCombo = false;
			bHasBufferedInput = false;

			// 몽타주 종료 델리게이트 바인딩
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &AAGSDCharacter::OnAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Stage.AttackMontage);
		}
	}
}

void AAGSDCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 입력 버퍼가 있다면 이미 ExecuteNextStage()에서 다음 공격이 시작되었을 것이므로 무시
	if (bInterrupted) return;

	// 현재 스테이지 데이터 참조
	if (CurrentComboData && CurrentComboData->Stages.IsValidIndex(CurrentStageIndex))
	{
			ResetAttackState();
	}
}

void AAGSDCharacter::StartRecovery(UAnimMontage* RecoveryMontage)
{
	if (!RecoveryMontage) return;

	bIsRecovering = true;
	bCanCombo = false; // 복귀 중에는 일반 콤보 창은 닫힘 (단, 새로운 입력을 통한 캔슬은 허용)
    
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	PlayAnimMontage(RecoveryMontage);

	// 복귀 완료 시 상태 최종 리셋을 위한 바인딩
	FOnMontageEnded RecoveryEndedDelegate;
	RecoveryEndedDelegate.BindUObject(this, &AAGSDCharacter::OnRecoveryFinished);
	AnimInstance->Montage_SetEndDelegate(RecoveryEndedDelegate, RecoveryMontage);
}

void AAGSDCharacter::EndJustGuardWindow()
{
	// 타이머에 의해 호출되면 이제 더 이상 저스트 가드 판정이 아님
	bIsJustGuardWindow = false;

	// (선택 사항) 로그를 남겨서 판정 종료를 확인해볼 수 있습니다.
	// UE_LOG(LogTemp, Log, TEXT("Just Guard Window Closed"));
}

void AAGSDCharacter::StartBlock()
{
	if (Mining) return;
	if (HoldingWeapon != EHoldingWeapon::Spear) return;
	
	bIsBlocking = true;
	PlayAnimMontage(BlockStartMontage);

	Mining = true;
	// 가드 시작 후 0.2초간 저스트 가드 판정 활성화
	bIsJustGuardWindow = true;
	GetWorldTimerManager().SetTimer(JustGuardTimerHandle, this, &AAGSDCharacter::EndJustGuardWindow, 0.2f, false);
}

void AAGSDCharacter::StopBlock()
{
	if (bIsBlocking)
	{
		bIsBlocking = false;
		StopAnimMontage(BlockStartMontage);
		Mining = false;
	}
}

void AAGSDCharacter::HandleJustGuardSuccess()
{
	// 1. 상태 초기화 (연속 성공 방지 및 타이머 정리)
	bIsJustGuardWindow = false;
	GetWorldTimerManager().ClearTimer(JustGuardTimerHandle);
	// 2. 사운드 재생
	if (JustGuardSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JustGuardSound, GetActorLocation());
	}
	// 3. 이펙트 스폰 (보통 창이나 방패 위치에 스폰하는 것이 좋음)
	if (JustGuardParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), JustGuardParticle, GetActorLocation());
	}
	// 4. (고급 기능) 역경직(Hit-Stop) 연출
	// 가드 성공의 쾌감을 위해 시간 배율을 아주 잠깐 동안 낮췄다 돌려주면 타격감이 훨씬 좋아집니다.
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
    
	FTimerHandle ResetTimeDilationTimer;
	GetWorldTimerManager().SetTimer(ResetTimeDilationTimer, FTimerDelegate::CreateLambda([this]()
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}), 0.05f, false); // 0.05초 뒤에 다시 원래 속도로
	UE_LOG(LogTemp, Warning, TEXT("Just Guard Success!"));
}

void AAGSDCharacter::ResetCombo()
{
	// 1. 모든 상태 플래그 초기화
	bIsAttacking = false;
	bIsRecovering = false;
	bCanCombo = false;
	bHasBufferedInput = false;
    
	// 2. 콤보 데이터 초기화
	CurrentStageIndex = -1;
	CurrentComboData = nullptr;

	// 3. (추가) 수비 시스템 관련 상태도 함께 초기화
	bIsJustGuardWindow = false;
	if (GetWorldTimerManager().IsTimerActive(JustGuardTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(JustGuardTimerHandle);
	}

	// 몽타주가 실행 중이라면 정지 (선택 사항)
	// GetMesh()->GetAnimInstance()->StopAllMontages(0.2f);

	UE_LOG(LogTemp, Log, TEXT("Combo System Reset"));
}

void AAGSDCharacter::OnHitReceived()
{
	if (bIsJustGuardWindow)
	{
		HandleJustGuardSuccess();
	}
	else
	{
		// 일반 피격 시 콤보 리셋 및 모든 공격 상태 강제 종료
		ResetCombo();
	}
}

void AAGSDCharacter::OnRecoveryFinished(UAnimMontage* Montage, bool bInterrupted)
{
	// 다른 공격에 의해 캔슬(Interrupted)된 게 아니라 자연 종료된 경우에만 상태 리셋
	if (!bInterrupted)
	{
		ResetAttackState();
	}
}

void AAGSDCharacter::ResetAttackState()
{
	// 선입력된 공격이 있고, 입력된 지 설정된 시간(AttackBufferDuration) 이내인 경우에만 유효한 것으로 판정
	float CurrentTime = GetWorld()->GetTimeSeconds();
	bool bShouldTriggerBufferedAttack = bHasBufferedInput && (CurrentTime - BufferedInputTime <= AttackBufferDuration);

	bIsAttacking = false;
	bIsRecovering = false;
	bCanCombo = false;
	bHasBufferedInput = false;
	CurrentStageIndex = -1;
	CurrentComboData = nullptr;
	Mining = false;

	// 공격 종료 시점에 선입력된 입력이 있다면 즉시 새로운 공격 실행
	if (bShouldTriggerBufferedAttack)
	{
		ProcessAttackInput();
	}
}

void AAGSDCharacter::ToggleLockOn()
{
	if (LockedTarget)
	{
		// 1. 이미 락온 중이라면 조준선 끄고 락온 해제
		SetLockOnMarkerState(LockedTarget, false);
		LockedTarget = nullptr;

		// 시야 차단 타이머 및 상태 초기화
		GetWorldTimerManager().ClearTimer(LineOfSightTimerHandle);
		bIsLineOfSightBlocked = false;

		// 모션 워프 타겟 제거
		if (MotionWarpingComponent)
		{
			MotionWarpingComponent->RemoveWarpTarget(FName("WarpTarget"));
		}
	}
	else
	{
		// 2. 락온 중이 아니라면 주변의 가장 가까운 적 탐색
		LockedTarget = FindNearestLockOnTarget();
		if (LockedTarget)
		{
			// 조준선 켜고 애니메이션 재생
			SetLockOnMarkerState(LockedTarget, true);
		}
	}

	OnLockOnStateChanged.Broadcast(LockedTarget != nullptr);
}

AActor* AAGSDCharacter::FindNearestLockOnTarget()
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		LockOnRadius,
		{ UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn) },
		AActor::StaticClass(),
		ActorsToIgnore,
		OverlappingActors
	);

	AActor* BestTarget = nullptr;
	float BestScore = FLT_MAX;

	FVector CameraLocation = GetFollowCamera()->GetComponentLocation();
	FVector CameraForward = GetFollowCamera()->GetForwardVector();

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag(FName("Enemy")))
		{
			// 1. 캐릭터와의 거리 계산
			float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Distance > LockOnRadius) continue;

			// 2. 카메라 전방 기준 시야각 오프셋(AngleOffset) 계산
			FVector DirToTarget = (Actor->GetActorLocation() - CameraLocation).GetSafeNormal();
			float Dot = FVector::DotProduct(CameraForward, DirToTarget);
			float AngleOffset = FMath::RadiansToDegrees(FMath::Acos(Dot));

			// 3. 시야각 120도 이내 필터링 (중앙선 기준 좌우 60도)
			if (AngleOffset <= 60.0f)
			{
				// 4. Normalized Score (가중치 1:1) 계산
				float NormDistance = Distance / LockOnRadius;
				float NormAngle = AngleOffset / 60.0f;
				float Score = (NormDistance * 1.0f) + (NormAngle * 1.0f);

				if (Score < BestScore)
				{
					BestScore = Score;
					BestTarget = Actor;
				}
			}
		}
	}
	return BestTarget;
}

void AAGSDCharacter::SwitchTargetLeft()
{
	SwitchTarget(true);
}

void AAGSDCharacter::SwitchTargetRight()
{
	SwitchTarget(false);
}

void AAGSDCharacter::SwitchTarget(bool bLookLeft)
{
	if (!LockedTarget) return;

	TArray<AActor*> OverlappingActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		LockOnRadius,
		{ UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn) },
		AActor::StaticClass(),
		ActorsToIgnore,
		OverlappingActors
	);

	AActor* NewTarget = nullptr;
	float MinYDiff = FLT_MAX;

	FTransform CameraTransform = GetFollowCamera()->GetComponentTransform();
	FVector CurrentTargetLocal = CameraTransform.InverseTransformPosition(LockedTarget->GetActorLocation());

	FVector CameraLocation = GetFollowCamera()->GetComponentLocation();
	FVector CameraForward = GetFollowCamera()->GetForwardVector();

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag(FName("Enemy")) && Actor != LockedTarget)
		{
			// 1. 시야각 120도 필터링
			FVector DirToTarget = (Actor->GetActorLocation() - CameraLocation).GetSafeNormal();
			float Dot = FVector::DotProduct(CameraForward, DirToTarget);
			float AngleOffset = FMath::RadiansToDegrees(FMath::Acos(Dot));

			if (AngleOffset > 60.0f) continue;

			// 2. 카메라 공간 상의 위치 계산 (Y축: Right)
			FVector EnemyLocal = CameraTransform.InverseTransformPosition(Actor->GetActorLocation());
			float YDiff = EnemyLocal.Y - CurrentTargetLocal.Y;

			if (bLookLeft)
			{
				// 좌측에 있는 적: 현재 타겟의 로컬 Y보다 작은 Y값을 가짐
				if (YDiff < 0.0f)
				{
					float AbsDiff = FMath::Abs(YDiff);
					if (AbsDiff < MinYDiff)
					{
						MinYDiff = AbsDiff;
						NewTarget = Actor;
					}
				}
			}
			else
			{
				// 우측에 있는 적: 현재 타겟의 로컬 Y보다 큰 Y값을 가짐
				if (YDiff > 0.0f)
				{
					float AbsDiff = FMath::Abs(YDiff);
					if (AbsDiff < MinYDiff)
					{
						MinYDiff = AbsDiff;
						NewTarget = Actor;
					}
				}
			}
		}
	}

	if (NewTarget)
	{
		// 마커 전환 및 락온 대상 변경
		SetLockOnMarkerState(LockedTarget, false);
		LockedTarget = NewTarget;
		SetLockOnMarkerState(LockedTarget, true);

		// 시야 차단 타이머 상태 초기화
		GetWorldTimerManager().ClearTimer(LineOfSightTimerHandle);
		bIsLineOfSightBlocked = false;

		OnLockOnStateChanged.Broadcast(true);
	}
}

void AAGSDCharacter::OnLineOfSightTimeout()
{
	if (LockedTarget && bIsLineOfSightBlocked)
	{
		ToggleLockOn();
	}
}

void AAGSDCharacter::UpdateMotionWarpTarget()
{
	if (MotionWarpingComponent)
	{
		if (!LockedTarget)
		{
			// 락온 타겟이 없을 때 모션 워프 타겟 제거
			MotionWarpingComponent->RemoveWarpTarget(FName("WarpTarget"));
			return;
		}

		FVector PlayerLoc = GetActorLocation();
		FVector TargetLoc = LockedTarget->GetActorLocation();

		// 1. 플레이어 캐릭터 전방 120도 시야 범위(좌우 60도) 내에 타겟이 있는지 검사
		FVector Forward = GetActorForwardVector();
		FVector DirToTarget = (TargetLoc - PlayerLoc).GetSafeNormal2D();
		float Dot = FVector::DotProduct(Forward, DirToTarget);
		float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(Dot));

		if (AngleDiff > MaxAngleDiff)
		{
			// 정면 시야각 120도를 벗어났다면(예: 뒤를 돌아서 공격 시 등), 모션 워프 타겟을 제거하여 급회전 현상 방지
			MotionWarpingComponent->RemoveWarpTarget(FName("WarpTarget"));
			return;
		}

		// 2. 플레이어 캐릭터가 락온 타겟을 바라보는 회전값 계산
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, TargetLoc);
		TargetRotation.Pitch = 0.f;
		TargetRotation.Roll = 0.f;

		// 수평(2D) 방향 및 거리 계산
		float ActualDistance = FVector::Dist2D(PlayerLoc, TargetLoc);

		FVector WarpLocation = PlayerLoc;
		const float AttackStopDistance = 150.0f; // 몬스터 전방 150.0f 남겨두기
		const float MaxWarpStep = 350.0f;        // 공격 시 순간이동 방지를 위한 최대 워프 전진 제한 거리

		if (ActualDistance > AttackStopDistance)
		{
			// 몬스터 전방 150.0f가 되는 목표 지점 계산
			FVector DesiredWarpLoc = TargetLoc - (DirToTarget * AttackStopDistance);
			
			// 해당 목표 지점까지의 워프 필요 거리
			float WarpDist = FVector::Dist2D(PlayerLoc, DesiredWarpLoc);
			if (WarpDist > MaxWarpStep)
			{
				// 최대 허용치만큼만 앞으로 전진 워프
				WarpLocation = PlayerLoc + (DirToTarget * MaxWarpStep);
			}
			else
			{
				WarpLocation = DesiredWarpLoc;
			}
		}
		else
		{
			// 이미 공격 사거리 이내(150.0f 이하)에 있다면 현재 위치 유지하며 회전만 보정
			WarpLocation = PlayerLoc;
		}

		// Z축 좌표는 플레이어의 현재 높이를 유지하여 땅밑 침하 또는 공중 부양 방지
		WarpLocation.Z = PlayerLoc.Z;

		// WarpTarget 업데이트
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			FName("WarpTarget"),
			WarpLocation,
			TargetRotation
		);
	}
}

void AAGSDCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAGSDCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// MouseSensitivity가 5일 때 (5/5 = 1)이 되어 기존 속도와 동일해집니다.
		float SensitivityMultiplier = MouseSensitivity / 5.0f;

		AddControllerYawInput(Yaw * SensitivityMultiplier);
		AddControllerPitchInput(Pitch * SensitivityMultiplier);
	}
}

void AAGSDCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AAGSDCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AAGSDCharacter::AddDamage(float addDamage)
{
	Damage += addDamage;
	if (PlayerStateWidget)
	{
		PlayerStateWidget->SetDamageText(Damage);
	}
}

void AAGSDCharacter::HealthRecovery(float amount)
{
	Health = FMath::Clamp(Health + amount, 0, MaxHealth);
	if (HealthBar) HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
}
//--------------

void AAGSDCharacter::SetMyPet_Implementation(AActor* NewPet)
{
	if (NewPet)
	{
		Pet = Cast<ABaseFlyingPet>(NewPet);
		bHasPet = true; // 이제 펫이 있다고 표시
	}
}

void AAGSDCharacter::MasterToPetBigConversation_Implementation(FName DialogueID)
{
	if ( Pet )
	{
		Pet->StartBigConversation(DialogueID);
	}
}

void AAGSDCharacter::DestroyPetBeforeTravel()
{
	if ( bHasPet && Pet )
	{
		Pet->Destroy();
		Pet = nullptr;
	}
}

void AAGSDCharacter::SpawnMyPetAfterTravel()
{
	if ( bHasPet )
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnMyPetAfterTravel"));
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		FVector SpawnLocation = GetActorLocation() + FVector(0.f, -180.f, 0.f); // 캐릭터 뒤쪽에 스폰
		FRotator SpawnRotation = FRotator::ZeroRotator;

		GetWorld()->SpawnActor<ABaseFlyingPet>(DefaultPetClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
}

// C:\Users\YJH\Documents\GitHub\AGSD\Source\AGSD\AGSDCharacter.cpp

void AAGSDCharacter::HandleEnemyDeadAndDropItem_Implementation(AActor* DeadActor)
{
	if (DeadActor && DeadActor->Implements<UItemDropInterface>())
	{
		// 1. 죽은 에너미에게서 고유 ID 받아오기 (데이터 테이블 Row Name)
		FName EnemyID = IItemDropInterface::Execute_GetItemDropTableEnemyID(DeadActor);
		
		// 데이터 테이블이 할당 안되거나 게임 인스턴스 할당이 없으면 종료
		if (!EnemyDropDataTable || !GI) return;

		// 2. 에너미 ID(Row Name)를 기반으로 해당 에너미의 모든 드롭 배열 가져오기
		FEnemyDropData* DropData = EnemyDropDataTable->FindRow<FEnemyDropData>(EnemyID, TEXT("EnemyDropInfo"));
		if (!DropData) return;

		// 3. 에너미가 설정한 아이템 배열을 순회
		for (const FEnemyDropItemInfo& ItemInfo : DropData->DropItems)
		{
			// 스폰할 클래스가 비어있으면 패스
			if (!ItemInfo.DropItemClass) continue;

			// 드롭 확률 체크 (0.0 ~ 1.0 사이의 값으로, 1.0이면 무조건 드롭, 0.5면 50% 확률로 드롭)
			if (FMath::FRand() > ItemInfo.DropChance) continue;
			
			// 4. 최초 1회성 드롭 여부 확인
			if (ItemInfo.bIsOneTimeDrop == true)
			{
				FString UniqueKey = EnemyID.ToString() + TEXT("_") + ItemInfo.DropItemClass->GetName();
				if (GI->AlreadyDroppedItems.Contains(UniqueKey))
				{
					continue;
				}
				else
				{
					GI->AlreadyDroppedItems.Add(UniqueKey);
				}
			}

			// --- 추가/수정된 로직: 드롭 개수 결정 ---
			int32 FinalDropAmount = ItemInfo.DropAmount;
			if (ItemInfo.bUseRandomDropAmount)
			{
				// Min과 Max 사이에서 무작위 개수 결정 (Min, Max 모두 포함)
				FinalDropAmount = FMath::RandRange(ItemInfo.DropAmountMin, ItemInfo.DropAmountMax);
			}

			// 5. 결정된 개수(FinalDropAmount)만큼 실제로 스폰시키기
			for (int32 i = 0; i < FinalDropAmount; ++i)
			{
				TSubclassOf<AActor> ItemClassToSpawn = ItemInfo.DropItemClass;
				
				// 적의 현재 위치
				FVector SpawnLocation = DeadActor->GetActorLocation();

				// 캐릭터 발 밑 근처에서 약간의 랜덤 위치 오프셋을 주면 아이템들이 겹치지 않아 보기 좋습니다.
				FVector RandomOffset = FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), 0.f);
				SpawnLocation += RandomOffset;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				GetWorld()->SpawnActor<AActor>(ItemClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
			}
		}
	}
}

void AAGSDCharacter::StopAnimMontage(UAnimMontage* AnimMontage)
{
	Super::StopAnimMontage(AnimMontage);
	
	//ResetAttackState();
	bIsAttacking = false;
	bIsRecovering = false;
	bCanCombo = false;
	bHasBufferedInput = false;
	CurrentStageIndex = -1;
	CurrentComboData = nullptr;
	Mining = false;
}

//--------------

void AAGSDCharacter::HandleLockOn(bool bLockOn)
{
	if (PC)
	{
		if (bLockOn)
		{
			UE_LOG(LogTemp, Warning, TEXT("LockOn True - Note: Manual lock-on logic might be needed if this should trigger a lock-on"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LockOn False - Releasing LockOn"));
			if (LockedTarget)
			{
				ToggleLockOn();
			}
		}
	}
}

void AAGSDCharacter::SetLockOnMarkerState(AActor* TargetActor, bool bActive)
{
	if (!TargetActor) return;

	TArray<UWidgetComponent*> WidgetComps;
	TargetActor->GetComponents<UWidgetComponent>(WidgetComps);

	for (UWidgetComponent* Comp : WidgetComps)
	{
		if (Comp && Comp->ComponentHasTag(FName("LockOnMarker")))
		{
			Comp->SetVisibility(bActive);

			UUserWidget* UserWidget = Comp->GetUserWidgetObject();
			if (UserWidget)
			{
				UFunction* AnimFunc = UserWidget->FindFunction(FName("PlayLockOnAnim"));
				if (AnimFunc)
				{
					struct FPlayLockOnAnimArgs
					{
						bool bPlay;
					};
					FPlayLockOnAnimArgs Args;
					Args.bPlay = bActive;
					
					UserWidget->ProcessEvent(AnimFunc, &Args);
				}
			}
			break;
		}
	}
}

void AAGSDCharacter::SkipTutorialPressed()
{
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	if (TutorialMapNames.Contains(CurrentLevelName))
	{
		OnSkipTutorialTriggered();
	}
}

void AAGSDCharacter::OnSkipTutorialTriggered_Implementation()
{
	// 페이드 아웃 시작 (투명도 0.0 -> 1.0)
	playFadeWidget(0.0f, 1.0f, 6.0f);

	if (FadeWidget)
	{
		// 중복 바인딩 방지 후 페이드 아웃 완료 이벤트 바인딩
		FadeWidget->OnFadeFinished.RemoveDynamic(this, &AAGSDCharacter::ExecuteTutorialSkipLevelTransition);
		FadeWidget->OnFadeFinished.AddDynamic(this, &AAGSDCharacter::ExecuteTutorialSkipLevelTransition);
	}
	else
	{
		// 페이드 위젯이 유효하지 않을 경우 즉시 이동 (예외 처리)
		ExecuteTutorialSkipLevelTransition();
	}
}

void AAGSDCharacter::ExecuteTutorialSkipLevelTransition()
{
	if (FadeWidget)
	{
		// 이벤트 바인딩 해제
		FadeWidget->OnFadeFinished.RemoveDynamic(this, &AAGSDCharacter::ExecuteTutorialSkipLevelTransition);
	}

	// 블루프린트에서 타겟 맵 이름이 설정되어 있다면 해당 맵으로 이동
	if (TargetSkipMapName != NAME_None)
	{
		UGameplayStatics::OpenLevel(this, TargetSkipMapName);
	}
	else
	{
		// 설정되지 않았을 경우 기본값
		UGameplayStatics::OpenLevel(this, FName("Farm_Sky_Island"));
	}
}
