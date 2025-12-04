// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTestCharacter.h"

// Sets default values
AEnemyTestCharacter::AEnemyTestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

