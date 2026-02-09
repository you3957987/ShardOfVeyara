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
	
	MoveForwardDuringRushAttack(DeltaTime);
	
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

void ABossBlackKnight::RushAttack()
{
	if ( RushAttackMontage )
	{
		PlayAnimMontage(RushAttackMontage);

		RushTargetLocation = TargetCharacter->GetActorLocation();
	}
	
}

void ABossBlackKnight::StartRush()
{
	if (TargetCharacter == nullptr) return;

	// 1. 목표 지점 계산
	FVector StartLoc = GetActorLocation();
	FVector TargetLoc = TargetCharacter->GetActorLocation();
	TargetLoc.Z = StartLoc.Z; // 높이는 보스 높이로 고정

	// 타겟 바로 앞(공격 사거리)까지만 가도록 보정
	FVector Direction = (TargetLoc - StartLoc).GetSafeNormal();
	float Dist = FVector::Dist(StartLoc, TargetLoc);
	float StopDist = 100.0f; // 100cm 앞에서 멈춤
    
	// 최종 목표 위치 확정
	RushTargetLocation = StartLoc + Direction * (Dist - StopDist);
	RushStartLocation = StartLoc;

	// 2. 시간 설정 (거리에 상관없이 무조건 0.5초 만에 도착)
	RushDuration = 0.2f;
	RushElapsedTime = 0.0f;

	// 3. 타겟 바라보기
	SetActorRotation(Direction.Rotation());

	// 4. 돌진 시작 플래그 ON
	bIsRushing = true;
    
	// (선택) 물리 효과 끄기: 돌진 중 다른 힘에 밀리지 않도록
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void ABossBlackKnight::MoveForwardDuringRushAttack(float DeltaTime)
{
	if ( !bIsRushing ) return;

	RushElapsedTime += DeltaTime;

	// 0.0 ~ 1.0 사이의 진행률 계산 (0이면 시작점, 1이면 도착점)
	float Alpha = FMath::Clamp(RushElapsedTime / RushDuration, 0.0f, 1.0f);

	// 현재 위치 계산 (선형 보간: Lerp)
	// 시작점과 목표점 사이를 Alpha 비율만큼 이동한 위치
	FVector NewLocation = FMath::Lerp(RushStartLocation, RushTargetLocation, Alpha);

	// 위치 적용 (Sweep: true로 해서 벽에 막히게 함)
	if (!SetActorLocation(NewLocation, true))
	{
		// 벽에 부딪혔다면 돌진 중단
		bIsRushing = false;
		return;
	}

	// 시간이 다 됐으면 종료
	if (Alpha >= 1.0f)
	{
		bIsRushing = false;
		UE_LOG(LogTemp, Warning, TEXT("Rush Finished!"));

		// (선택) 물리 효과 복구
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		
		// 도착 후 처리 (예: 공격 판정, 몽타주 종료 등)
	}
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
