#include "EnemyBoss/MagicSwordMan/BossMagicSwordMan.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h" 
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

ABossMagicSwordMan::ABossMagicSwordMan()
{
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));
}

void ABossMagicSwordMan::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UCapsuleComponent*> CapsuleCollisionComps;
	GetComponents<UCapsuleComponent>(CapsuleCollisionComps);
	
	// 반복문 돌면서 태그 확인
	for (UCapsuleComponent* Capsule : CapsuleCollisionComps)
	{
		if (Capsule && Capsule->ComponentHasTag(TEXT("Weapon"))) // "Weapon" 태그를 가진 콜리전 스피어를 찾습니다.)))
		{
			WeaponCollision = Capsule;
			WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ABossMagicSwordMan::OnBeginOverlapWeaponCollisionSphere);
			// 콜리전 끄기
			WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break; // 찾았으니 루프 종료
		}
	}
}

void ABossMagicSwordMan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

float ABossMagicSwordMan::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if ( bIsGuarding )
	{
		DamageWhileGuarding += DamageAmount;

		// 아직 리액션 대미지에 도달하지 않았으면 가드 몽타주 재생
		if ( DamageWhileGuarding < MaxDamageToReaction && GuardMontage )
		{
			PlayAnimMontage(GuardMontage);
		}
		
		return 0.f; // 대미지 무효화
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABossMagicSwordMan::OnBeginOverlapWeaponCollisionSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ( AttackDamage == 0.f ) return; // 대미지가 0이면 피격 처리 안 함
	
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		// 어택 대미지 로그 
		UE_LOG(LogTemp, Warning, TEXT("Boss Attack Damage : %f"), AttackDamage);

		// 대미지 적용 ( 어택 대미지는 공격 전 가드 공격인지 아님 일반 공격인지에 따라 각각 함수에서 설정 )
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(),
			this, UDamageType::StaticClass());

		if ( AttackType == EMagicSwordManAttackType::JumpUpAttack )
		{
			// 플레이어 캐릭터 캐스팅
			ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);
			if (HitCharacter)
			{
				HitCharacter->LaunchCharacter(FVector(0.f, 0.f, 1180.f), false, true);
				bSuccessJumpUpAttack = true; // 띄우기 성공 여부를 true로 설정
			}
		}
		else if ( AttackType == EMagicSwordManAttackType::AirAttack )
		{
			// 공중 공격 피격 시 로직 추가
			ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);
			if (HitCharacter)
			{
				//HitCharacter->LaunchCharacter(FVector(0.f, 0.f, 10.f), false, true);
			}
		}

		// 다시 콜리전 끄기
		if ( WeaponCollision ) WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttackDamage = 0.f; // 대미지 초기화
	}
}

void ABossMagicSwordMan::AttackStart_WeaponCollision()
{
	if ( WeaponCollision ) WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABossMagicSwordMan::AttackEnd_WeaponCollision()
{
	if ( WeaponCollision ) WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// 공격 전에 한번씩 호출
void ABossMagicSwordMan::UpdateMotionWarpTarget()
{
	if (MotionWarpingComponent && TargetCharacter)
	{
		FVector BossLocation = GetActorLocation();
		FVector TargetLocation = TargetCharacter->GetActorLocation();

		// 1. 보스에서 타겟을 바라보는 회전값 계산
		// 단순히 TargetCharacter->GetActorRotation()을 쓰면 플레이어의 등 뒤를 보게 됩니다.
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(BossLocation, TargetLocation);
        
		// 2. 평면 회전만 원한다면 Pitch와 Roll은 0으로 고정 (Ignore Z Axis와 같은 효과)
		LookAtRotation.Pitch = 0.f;
		LookAtRotation.Roll = 0.f;

		// 3. 계산된 회전값으로 워프 타겟 업데이트
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			FName("WarpTarget"), 
			TargetLocation, 
			LookAtRotation
		);
		
		DrawDebugSphere(
			GetWorld(),
			TargetLocation,   // 위치
			50.0f,            // 반지름 (크기)
			12,               // 세그먼트 (구의 부드러움)
			FColor::Purple,      // 색상
			false,            // 지속 여부 (true면 영구 지속)
			2.0f,             // 화면에 표시될 시간 (초)
			0,                // 깊이 우선순위
			2.0f              // 선 두께
		);
	}
}

void ABossMagicSwordMan::UpdateMotionWarpTargetToFront()
{
	if (MotionWarpingComponent && TargetCharacter)
	{
		FVector BossLocation = GetActorLocation();
		FVector TargetLocation = TargetCharacter->GetActorLocation();

		// 1. 보스에서 타겟으로 향하는 방향 벡터 구하기 (Unit Vector)
		FVector DirectionToTarget = (TargetLocation - BossLocation).GetSafeNormal();

		// 2. 타겟 위치에서 보스 쪽으로 80만큼 뺀 위치 계산
		// 수식: 타겟위치 - (보스->타겟방향 * 80)
		// 이렇게 하면 타겟 바로 앞 80 거리의 지점이 구해집니다.
		const float FrontDistance = 150.0f;
		FVector WarpLocation = TargetLocation - (DirectionToTarget * FrontDistance);

		// 3. 회전값 계산 (보스가 타겟을 바라보도록 설정)
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(BossLocation, TargetLocation);
		LookAtRotation.Pitch = 0.f;
		LookAtRotation.Roll = 0.f;

		// 4. 워프 타겟 업데이트
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
			FName("WarpTarget"),
			WarpLocation,
			LookAtRotation
		);

		// 계산된 위치에 디버그 구체 그리기 (초록색)
		DrawDebugSphere(
			GetWorld(),
			WarpLocation,
			50.0f,
			12,
			FColor::Purple,
			false,
			2.0f,
			0,
			2.0f
		);
	}
}

void ABossMagicSwordMan::SetAttackDamage(float DamageToApply)
{
	AttackDamage = DamageToApply;
}

UAnimMontage* ABossMagicSwordMan::StartCloseAttack()
{
	if (CloseAttackMontages.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, CloseAttackMontages.Num() - 1);
		
		if (CloseAttackMontages[RandomIndex])
		{
			PlayAnimMontage(CloseAttackMontages[RandomIndex]);
			AttackType = EMagicSwordManAttackType::SimpleAttack;
			return CloseAttackMontages[RandomIndex];
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("array is EMPTY"));
	return nullptr;
}

UAnimMontage* ABossMagicSwordMan::StartDashAttack()
{	
	if (DashAttackMontages.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, DashAttackMontages.Num() - 1);
		
		if (DashAttackMontages[RandomIndex])
		{
			PlayAnimMontage(DashAttackMontages[RandomIndex]);
			AttackType = EMagicSwordManAttackType::SimpleAttack;
			return DashAttackMontages[RandomIndex];
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("array is EMPTY"));
	return nullptr;
}

UAnimMontage* ABossMagicSwordMan::StartCloseJumpUpAttack()
{
	if (CloseJumpUpAttackMontage)
	{
		PlayAnimMontage(CloseJumpUpAttackMontage);
		AttackType = EMagicSwordManAttackType::JumpUpAttack;
		// 플라잉 모드로 전환
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		bSuccessJumpUpAttack = false; // 공격 시작 시점에는 띄우기 성공 여부를 false로 초기화
		BlackboardComp->SetValueAsBool("bAirAttack", false);
		return CloseJumpUpAttackMontage;
	}
	return nullptr;
}

UAnimMontage* ABossMagicSwordMan::StartDashJumpUpAttack()
{
	if (DashJumpUpAttackMontage)
	{
		PlayAnimMontage(DashJumpUpAttackMontage);
		AttackType = EMagicSwordManAttackType::JumpUpAttack;
		// 플라잉 모드로 전환
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		bSuccessJumpUpAttack = false; // 공격 시작 시점에는 띄우기 성공 여부를 false로 초기화
		BlackboardComp->SetValueAsBool("bAirAttack", false);
		return DashJumpUpAttackMontage;
	}
	return nullptr;
}

void ABossMagicSwordMan::JumpUpAttackCheck()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	if ( bSuccessJumpUpAttack == true )
	{
		// 띄우기 성공 시 공중 공격 패턴으로 전환
		BlackboardComp->SetValueAsBool("bAirAttack", true);
	}
}

UAnimMontage* ABossMagicSwordMan::StartAirAttack()
{
	if (AirAttackMontage)
	{
		PlayAnimMontage(AirAttackMontage);
		AttackType = EMagicSwordManAttackType::AirAttack;
		
		BossGravityScaleBeforeAirAttack = GetCharacterMovement()->GravityScale;
		// 보스랑 타깃 캐릭터의 중력 설정
		GetCharacterMovement()->GravityScale = 0.78f;

		// 타깃 캐릭터의 중력 스케일 조절
		if (TargetCharacter)
		{
			// TargetCharacter가 ACharacter 타입인지 확인 후 캐스팅
			if (ACharacter* CastedTarget = Cast<ACharacter>(TargetCharacter))
			{
				TargetCharacterGravityScaleBeforeAirAttack = CastedTarget->GetCharacterMovement()->GravityScale;
				
				CastedTarget->GetCharacterMovement()->GravityScale = 0.75f;
				
			}
		}
		return AirAttackMontage;
	}
	return nullptr;
}

void ABossMagicSwordMan::AirAttackEnd()
{
	// 공중 공격이 끝났을 때 중력 스케일을 원래대로 복구
	GetCharacterMovement()->GravityScale = BossGravityScaleBeforeAirAttack;	
	if (TargetCharacter)
	{
		if (ACharacter* CastedTarget = Cast<ACharacter>(TargetCharacter))
		{
			CastedTarget->GetCharacterMovement()->GravityScale = TargetCharacterGravityScaleBeforeAirAttack;
		}
	}
}

UAnimMontage* ABossMagicSwordMan::StartJumpAttack()
{
	if (JumpAttackMontages.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, JumpAttackMontages.Num() - 1);
		
		if (JumpAttackMontages[RandomIndex])
		{
			PlayAnimMontage(JumpAttackMontages[RandomIndex]);
			AttackType = EMagicSwordManAttackType::SimpleAttack;
			return JumpAttackMontages[RandomIndex];
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("array is EMPTY"));
	return nullptr;
}

void ABossMagicSwordMan::JumpStart()
{
	LaunchCharacter( FVector(0.f, 0.f, 800.f), false, true);
}

#if WITH_EDITOR
void ABossMagicSwordMan::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABossMagicSwordMan, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			
		}
		else
		{
			
		}
	}
}
#endif