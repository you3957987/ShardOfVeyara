#include "EnemyBoss/Worm/BossWorm.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyProjectile/BaseEnemyProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABossWorm::ABossWorm()
{
	// 최대 걷기 속도 제로로 해서 움직이지 않도록 설정
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;
	
	UnBurrowAttackCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("UnBurrowAttackCollisionSphere"));
	UnBurrowAttackCollisionSphere->SetupAttachment(GetRootComponent());
	UnBurrowAttackCollisionSphere->ShapeColor = FColor::Blue;
	UnBurrowAttackCollisionSphere->SetVisibility(false);
	UnBurrowAttackCollisionSphere->SetHiddenInGame(false);
	// 콜리전 오버랩만 감지하도록 설정
	UnBurrowAttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	RangedAttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RangedAttackPoint"));
	RangedAttackPoint->SetupAttachment(RootComponent); // 루트 컴포넌트
}

void ABossWorm::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<USphereComponent*> SphereComps;
	GetComponents<USphereComponent>(SphereComps);
	
	// 반복문 돌면서 태그 확인
	for (USphereComponent* Sphere : SphereComps)
	{
		if (Sphere && Sphere->ComponentHasTag(TEXT("Attack"))) // "Attack" 태그가 있는 SphereComponent를 찾습니다.
		{
			AttackCollisionSphere = Sphere;
			AttackCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABossWorm::OnBeginOverlapAttackCollisionSphere);
			// 콜리전 끄기
			AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break; // 찾았으니 루프 종료
		}
	}
	
}

void ABossWorm::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if ( bIsBurrowing == true )
	{
		// 체력바 위젯 숨기기
		if ( HealthBarWidget ) HealthBarWidget->SetVisibility(false);
	}
	else
	{
		// 체력바 위젯 보이기
		if ( HealthBarWidget ) HealthBarWidget->SetVisibility(true);
	}
}

float ABossWorm::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABossWorm::OnBeginOverlapAttackCollisionSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack Sphere Overlapped with Player!"));

		// 대미지 적용 ( 어택 대미지는 공격 전 가드 공격인지 아님 일반 공격인지에 따라 각각 함수에서 설정 )
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(),
			this, UDamageType::StaticClass());

		if ( AttackDamage == LungeAttackDamage )
		{
			ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
			
			const float PushForce = 2000.0f;       // 밀어내는 힘
			const float PushUpwardForce = 400.0f;  // 띄우는 힘

			// 밀어낼 방향 계산 (보스 -> 플레이어)
			FVector PushDirection = PlayerCharacter->GetActorLocation() - GetActorLocation();
			PushDirection.Z = 0; 
			PushDirection.Normalize();

			// 밀어낼 속도 계산
			const FVector LaunchVelocity = PushDirection * PushForce + FVector(0.f, 0.f, PushUpwardForce);

			// 플레이어 캐릭터를 밀어냄
			PlayerCharacter->LaunchCharacter(LaunchVelocity, true, true);
		}
		
		// 다시 콜리전 끄기
		if ( AttackCollisionSphere ) AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABossWorm::AttackStart_AttackCollisionSphere()
{
	if ( AttackCollisionSphere ) AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABossWorm::AttackEnd_AttackCollisionSphere()
{
	if ( AttackCollisionSphere ) AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABossWorm::FinishBurrow()
{
	// 캡슐 콜리전 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bIsBurrowing = true; // 땅 아래로 들어가는 몽타주가 끝나는 시점에 땅파는 상태를 true로 변경
	
	// 딜레이 설정
	if ( BlackboardComp )
	{
		BlackboardComp->SetValueAsFloat("AttackDelay", BurrowDelay); // 행동 딜레이 설정
	}
	
	// 타겟 위치 세팅하는 함수 == BurrowDelay 에서 2초 정도 남았을 때 호출되어 타겟 위치 세팅
	float TargetSetDelay = BurrowDelay - 2.0f;
	
	// 만약 BurrowDelay가 2초보다 짧다면 즉시 호출하거나 아주 짧은 딜레이로 설정
	if (TargetSetDelay < 0.0f)
	{
		TargetSetDelay = 0.1f;
	}

	GetWorld()->GetTimerManager().SetTimer(
		UnBurrowTargetTimerHandle, 
		this, 
		&ABossWorm::SetUnBurrowTargetLocation, 
		TargetSetDelay, 
		false
	);
}

void ABossWorm::FinishUnburrow()
{
	// 캡슐 콜리전 활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	bIsBurrowing = false; // 땅에서 나오는 몽타주가 끝나는 시점에 땅파는 상태를 false로 변경
	
	// 공격 딜레이 설정
	if ( BlackboardComp )
	{
		BlackboardComp->SetValueAsFloat("AttackDelay", UnBurrowAttackDelay); // 행동 딜레이 설정
	}
}


void ABossWorm::NormalAttack()
{
	if ( NormalAttackMontage )
	{
		PlayAnimMontage(NormalAttackMontage);

		AttackDamage = NormalAttackDamage; // 공격 대미지 설정
		
		bFocusPlayerAfterAttack = false; // 공격 중에는 포커스 비활성화
		
		if ( BlackboardComp )
		{
			BlackboardComp->SetValueAsFloat("AttackDelay", NormalAttackDelay); // 행동 딜레이 설정
		}
	}
}

void ABossWorm::LungeAttack()
{
	if ( LungeAttackMontage )
	{
		PlayAnimMontage(LungeAttackMontage);

		AttackDamage = LungeAttackDamage; // 공격 대미지 설정
		
		bFocusPlayerAfterAttack = false;  
		
		if ( BlackboardComp )
		{
			BlackboardComp->SetValueAsFloat("AttackDelay", LungeAttackDelay); // 행동 딜레이 설정
		}
	}
}

void ABossWorm::Burrow()
{
	if ( BurrowMontage )
	{
		PlayAnimMontage(BurrowMontage);
		BlackboardComp->SetValueAsBool( TEXT("bIsBurrowing"), true);
	}
}

void ABossWorm::SetUnBurrowTargetLocation()
{
	if (TargetCharacter)
	{
		// 1. 타겟의 현재 위치 (캡슐 중심)
		UnBurrowTargetLocation = TargetCharacter->GetActorLocation();

		// 타겟의 캡슐 컴포넌트 가져오기 (타겟 발바닥 위치 계산용)
		UCapsuleComponent* TargetCapsule = TargetCharacter->GetCapsuleComponent();
		float TargetHalfHeight = 0.f;
		if (TargetCapsule)
		{
			TargetHalfHeight = TargetCapsule->GetScaledCapsuleHalfHeight();
		}

		// 내(보스) 캡슐 컴포넌트 가져오기 (내 중심 위치 보정용)
		UCapsuleComponent* MyCapsule = GetCapsuleComponent();
		float MyHalfHeight = 0.f;
		if (MyCapsule)
		{
			MyHalfHeight = MyCapsule->GetScaledCapsuleHalfHeight();
		}

		// 공식: 타겟 중심 - 타겟 키(발바닥으로 내림) + 내 키(내 중심으로 올림)
		UnBurrowTargetLocation.Z = (UnBurrowTargetLocation.Z - TargetHalfHeight) + MyHalfHeight;

		// 2. 먼저 계산된 위치로 이동
		SetActorLocation(UnBurrowTargetLocation);
		
		// 3. 회전 설정 (보스 현재 위치에서 타겟을 바라보는 회전 계산)
		// 주의: 이미 위치를 옮겼으므로 단순히 TargetCharacter를 바라보면 됨
		const FRotator LookAtRotation =
			UKismetMathLibrary::FindLookAtRotation(UnBurrowTargetLocation, TargetCharacter->GetActorLocation());

		// 수평으로만 바라보도록 Yaw 값만 사용합니다.
		const FRotator SpawnRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);

		SetActorRotation(SpawnRotation);
	}
	else
	{
		// 타겟이 없으면 제자리
		UnBurrowTargetLocation = GetActorLocation();
	}
}

void ABossWorm::Unburrow()
{
	if ( UnburrowMontage )
	{
		PlayAnimMontage(UnburrowMontage);
		BlackboardComp->SetValueAsBool( TEXT("bIsBurrowing"), false);
	}
	
	// --- 여기서부터 즉시 오버랩 검사 및 밀치기 로직 ---
	if (UnBurrowAttackCollisionSphere)
	{
		TArray<AActor*> OverlappingActors;
		UnBurrowAttackCollisionSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

		for (AActor* Actor : OverlappingActors)
		{
			if (Actor && Actor != this && Actor->ActorHasTag(FName("Player")))
			{
				ACharacter* PlayerCharacter = Cast<ACharacter>(Actor);
				if (PlayerCharacter)
				{
					const float PushForce = 2000.0f;       // 밀어내는 힘
					const float PushUpwardForce = 400.0f;  // 띄우는 힘

					// 밀어낼 방향 계산 (보스 -> 플레이어)
					FVector PushDirection = PlayerCharacter->GetActorLocation() - GetActorLocation();
					PushDirection.Z = 0; 
					PushDirection.Normalize();

					// 밀어낼 속도 계산
					const FVector LaunchVelocity = PushDirection * PushForce + FVector(0.f, 0.f, PushUpwardForce);

					// 플레이어 캐릭터를 밀어냄
					PlayerCharacter->LaunchCharacter(LaunchVelocity, true, true);
					
					// 대미지 적용
					UGameplayStatics::ApplyDamage(PlayerCharacter, UnBurrowAttackDamage, GetController(),
						this, UDamageType::StaticClass());
					
					// 로그 출력 (필요시 주석 해제)
					UE_LOG(LogTemp, Warning, TEXT("Player Hit At Unburrow Attack!"));
				}
			}
		}
	}
}

void ABossWorm::RangedAttack()
{
	if ( RangedAttackMontage )
	{
		PlayAnimMontage( RangedAttackMontage);
		
		bFocusPlayerAfterAttack = false; // 공격 중에는 포커스 비활성화
		
		if ( BlackboardComp )
		{
			BlackboardComp->SetValueAsFloat("AttackDelay", RangedAttackDelay); // 행동 딜레이 설정
		}
	}
}

void ABossWorm::ShootProjectile()
{
	// 로그
	UE_LOG(LogTemp, Warning, TEXT("ShootProjectile called!"));
	if (ProjectileClass && RangedAttackPoint && TargetCharacter)
	{
		const FVector SpawnLocation = RangedAttackPoint->GetComponentLocation();
		FVector TargetLocation = TargetCharacter->GetActorLocation();
		const UCapsuleComponent* TargetCapsule = TargetCharacter->GetCapsuleComponent();
		if (TargetCapsule)
		{
			TargetLocation.Z -= TargetCapsule->GetScaledCapsuleHalfHeight();
		}

		FVector LaunchVelocity;
		const float ProjectileSpeed = 1500.0f; // 발사체 속도 (너무 느리면 도달 못 할 수 있음)

		// SuggestProjectileVelocity 사용 (일반 버전)
		// bHighArc = false (Low Arc, 직선에 가까운 곡사)
		bool bHaveSolution = UGameplayStatics::SuggestProjectileVelocity(
			this,
			LaunchVelocity,
			SpawnLocation,
			TargetLocation,
			ProjectileSpeed,
			false, // bHighArc: false로 설정하여 낮게 날아가도록 함
			0.0f,  // Radius: 충돌 검사 반지름 (0이면 검사 안함)
			0.0f,  // Gravity Override: 0이면 월드 중력 사용
			ESuggestProjVelocityTraceOption::DoNotTrace // 트레이스 옵션
		);

		// 해결책을 못 찾았을 경우 (속도가 부족하거나 각이 안 나옴)
		if (!bHaveSolution)
		{
			// 그냥 타겟 방향으로 직사로 쏨 + 약간 위로 보정
			FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
			Direction.Z += 0.2f; // 약간 위로 던져서 중력 보상 흉내
			LaunchVelocity = Direction.GetSafeNormal() * ProjectileSpeed;
		}

		// 회전값은 속도 벡터의 방향으로 설정
		const FRotator SpawnRotation = LaunchVelocity.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		UWorld* World = GetWorld();
		if (World)
		{
			// 발사체 스폰
			ABaseEnemyProjectile* SpawnedProjectile =
				World->SpawnActor<ABaseEnemyProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

			if (SpawnedProjectile)
			{
				UProjectileMovementComponent* ProjMoveComp =
					SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();
				if (ProjMoveComp)
				{
					// 속도 적용
					ProjMoveComp->Velocity = LaunchVelocity;
					// [중요] 발사체의 속도가 여기서 설정한 Velocity 크기로 강제 조정되도록 설정할 수도 있음
					// ProjMoveComp->InitialSpeed = LaunchVelocity.Size();
					// ProjMoveComp->MaxSpeed = LaunchVelocity.Size();
				}
			}
		}
	}
}


#if WITH_EDITOR
void ABossWorm::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABossWorm, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( UnBurrowAttackCollisionSphere ) UnBurrowAttackCollisionSphere->SetVisibility(true);
		}
		else
		{
			if ( UnBurrowAttackCollisionSphere ) UnBurrowAttackCollisionSphere->SetVisibility(false);
		}
	}
}
#endif
