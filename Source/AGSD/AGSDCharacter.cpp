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
	IInteraction::Execute_Interact(CurrentInteractableActor);
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

void AAGSDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CanInteract)
	{
		CurrentInteractableActor = nullptr;
		return;
	}
	
	float MinDistance = FLT_MAX;

	const FVector PlayerLocation = GetActorLocation();

	AActor* MinDistanceActor = nullptr;
	
	for (AActor* CurrentActor : InteractableActorsInRange)
	{
		if (!CurrentActor) continue;

		const float DistanceSq = FVector::DistSquared(PlayerLocation, CurrentActor->GetActorLocation());
		if (DistanceSq < MinDistance)
		{
			MinDistance = DistanceSq;
			MinDistanceActor = CurrentActor;
		}
	}
	if (MinDistanceActor != nullptr && CurrentInteractableActor != MinDistanceActor)
	{
		CurrentInteractableActor = MinDistanceActor;
		IInteraction::Execute_ShowWidget(CurrentInteractableActor, this);
	}
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

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

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
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
