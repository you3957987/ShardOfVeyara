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
#include "AGSD.h"
#include "HeartProgressBar.h"
#include "Components/WidgetComponent.h"

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

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
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
	if (!CanInteract)
	{
		CurrentInteractableActor = nullptr;
		return;
	}
	if (!IsValid(CurrentInteractableActor)) return;
	if (!CurrentInteractableActor->Implements<UInteraction>()) return;
	IInteraction::Execute_Interact(CurrentInteractableActor, this);
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

	PC = Cast<AAGSDPlayerController>(GetController());

	if (HealthBarWidget)
	{
		UE_LOG(LogTemp, Display, TEXT("HealthBarWidget is %s"), *HealthBarWidget->GetName());
		HealthBar = Cast<UHeartProgressBar>(HealthBarWidget->GetUserWidgetObject());
		if (HealthBar)
		{
			UE_LOG(LogTemp, Display, TEXT("HealthBarWidget is %s"), *HealthBar->GetName());
			HealthBar->SetPercent(Health / MaxHealth);
		}
	}
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

float AAGSDCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Warning, TEXT("Player Take Damage : %f"), DamageToApply);

	if ( DamageToApply > 0.f )
	{
		Health -= DamageToApply;
		if (HealthBar) HealthBar->SetPercent(Health / MaxHealth);
		if ( Health <= 0.f )
		{
			//Die();
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
				EnhancedSubsystem->AddMappingContext(IMC_Farmer, 1);
			}
		}
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAGSDCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAGSDCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAGSDCharacter::Look);

		//G 키를 누를 때 TryInteract 함수를 호출하도록 바인딩
		EnhancedInputComponent->BindAction(Interaction, ETriggerEvent::Triggered, this, &AAGSDCharacter::TryInteract);
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
	if (MovementVector.Y > 0.0f) HandleAttackInput(FName("Forward"));
	if (Mining) return;
	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAGSDCharacter::Jump()
{
	if (Mining) return;
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
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
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
