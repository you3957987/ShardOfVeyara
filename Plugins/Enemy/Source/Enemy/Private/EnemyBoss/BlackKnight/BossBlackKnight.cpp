#include "EnemyBoss/BlackKnight/BossBlackKnight.h"

#include "GameFramework/CharacterMovementComponent.h"

ABossBlackKnight::ABossBlackKnight()
{
	GetCharacterMovement()->bOrientRotationToMovement = false; 
}

void ABossBlackKnight::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABossBlackKnight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * 300.f, 100.f,
		FColor::Red, false, -1.f, 0, 5.f);
	
	// BS 이동 테스트용 코드
	/*
	static float DirectionTimer = 0.0f;
	DirectionTimer += DeltaTime;

	// 1. 이동 속도 강제 설정 (> 300)
	GetCharacterMovement()->MaxWalkSpeed = 200.0f; 

	// 2. 회전 고정: 이동 방향으로 몸을 돌리면 Angle이 항상 0이 되므로, 강제로 앞을 보게 고정합니다.
	GetCharacterMovement()->bOrientRotationToMovement = false; 
	SetActorRotation(FRotator(0.f, 0.f, 0.f)); 

	// 3. 2초마다 방향 전환 로직
	int32 State = (int32)(DirectionTimer / 2.0f) % 4; 
	FVector MoveDirection = FVector::ZeroVector;

	switch (State)
	{
	case 0:
		MoveDirection = FVector::ForwardVector; // Angle 0
		UE_LOG(LogTemp, Log, TEXT("Testing: Forward (0)"));
		break;
	case 1:
		MoveDirection = FVector::RightVector;   // Angle 90
		UE_LOG(LogTemp, Log, TEXT("Testing: Right (90)"));
		break;
	case 2:
		MoveDirection = FVector::BackwardVector;// Angle 180 (or -180)
		UE_LOG(LogTemp, Log, TEXT("Testing: Backward (180)"));
		break;
	case 3:
		MoveDirection = FVector::LeftVector;    // Angle -90
		UE_LOG(LogTemp, Log, TEXT("Testing: Left (-90)"));
		break;
	}

	// 결정된 방향으로 이동 입력 추가
	AddMovementInput(MoveDirection, 1.0f);

	*/
	
	/*
	if (bIsAttacking == true ) // 공격 중일 때
	{
		TArray<AActor*> OverlappingActors;
		// AttackRangePointSphere와 겹치는 모든 액터를 가져옵니다.
		AxeMeshComponent->GetOverlappingActors(OverlappingActors);
		
		for (AActor* OverlappingActor : OverlappingActors)
		{
			// 액터가 유효하고 "Player" 태그를 가지고 있으며, 아직 공격한 목록에 없는지 확인합니다.
			if (OverlappingActor && OverlappingActor->ActorHasTag(FName("Player"))
				&& !HittedActors.Contains(OverlappingActor))
			{
				// 공격 로그를 출력합니다.
				UE_LOG(LogTemp, Warning, TEXT("Attack Hit Detected on: %s"), *OverlappingActor->GetName());

				// 공격한 목록에 추가하여 중복 피해를 방지합니다.
				HittedActors.Add(OverlappingActor);

				bIsAttacking = false; // 공격 상태를 종료합니다.

				
			}
		}
	}
	*/
}

#if WITH_EDITOR
void ABossBlackKnight::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABossBlackKnight, bDebugMode))
	{
		
	}
}
#endif
