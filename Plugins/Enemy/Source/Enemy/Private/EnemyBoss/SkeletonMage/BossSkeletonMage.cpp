#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "EnemyProjectile/BaseEnemyProjectile.h"
#include "EnemyProjectile/GroundAttackProjectile.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h" // 헤더 파일 추가
#include "Components/CapsuleComponent.h" 
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABossSkeletonMage::ABossSkeletonMage()
{
	SummonPointOne = CreateDefaultSubobject<USceneComponent>(TEXT("SummonPointOne"));
	SummonPointOne->SetupAttachment(GetRootComponent());

	SummonPointTwo = CreateDefaultSubobject<USceneComponent>(TEXT("SummonPointTwo"));
	SummonPointTwo->SetupAttachment(GetRootComponent());

	PushAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PushAreaSphere"));
	PushAreaSphere->SetupAttachment(GetRootComponent());
}

void ABossSkeletonMage::BeginPlay()
{
	Super::BeginPlay();

	// 이 액터에 속한 모든 USceneComponent를 가져옵니다.
	TArray<USceneComponent*> SceneComponents;
	GetComponents<USceneComponent>(SceneComponents);

	// 이름으로 FireBallSpawnPoint 컴포넌트를 찾습니다.
	for (USceneComponent* Component : SceneComponents)
	{
		if (Component->GetFName() == TEXT("FireBallSpawnPoint"))
		{
			FireBallSpawnPoint = Component;
			break; // 찾았으면 반복 종료
		}
	}
}

void ABossSkeletonMage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceTargetCharacterForGroundAttackEffect(DeltaTime);
	
	HandleGravityAttack(DeltaTime);
	
	if (bIsAttacking == true ) // 공격 중일 때
	{
		TArray<AActor*> OverlappingActors;
		// 겹치는 모든 액터를 가져옵니다.
		PushAreaSphere->GetOverlappingActors(OverlappingActors);
		
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

				// --- 플레이어 밀치기 효과 시작 ---
				ACharacter* PlayerCharacter = Cast<ACharacter>(OverlappingActor);
				if (PlayerCharacter)
				{
					// 1. 밀어낼 방향 계산 (보스 -> 플레이어)
					FVector PushDirection = PlayerCharacter->GetActorLocation() - GetActorLocation();
					PushDirection.Z = 0; // 수평 방향으로만 밀도록 Z값을 0으로 설정
					PushDirection.Normalize();

					// 2. 밀어낼 속도 계산 (방향 * 힘 + 위로 띄우는 힘)
					const FVector LaunchVelocity = PushDirection * PushForce + FVector(0.f, 0.f, PushUpwardForce);

					// 3. 플레이어 캐릭터를 밀어냄
					// bXYOverride와 bZOverride를 true로 설정하여 현재 속도를 무시하고 새로운 속도를 적용합니다.
					PlayerCharacter->LaunchCharacter(LaunchVelocity, true, true);
				}
				// --- 플레이어 밀치기 효과 끝 ---
				
				UEnemyLogManager::EnemyLog(EEnemyLogType::SkeletonMage, FString::Printf(TEXT("[스켈레톤 메이지] 실드 캐스트 적중으로 플레이어 밀치기")));
				
				// 이 아래에 이제 대미지 넣는거 추가 가능
			}
		}
	}
}

void ABossSkeletonMage::PlayTeleportMontage(const FVector& Destination)
{
	if ( BlackboardComp == nullptr ) return;
	
	BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.TeleportDelay); // 텔포후 행동 딜레이 설정

	TeleportDestination = Destination;
	
	if ( TeleportMontage )
	{
		PlayAnimMontage(TeleportMontage);
		
		// 현재 위치에 텔레포트 입장 이펙트 생성
		if ( TeleportInEffect ) SpawnTeleportEffectAtLocation(GetActorLocation(), TeleportInEffect); 
	}
}

// 애님 노티파이에서 호출할 함수
void ABossSkeletonMage::TeleportMoveToNextPoint()
{
	if (TeleportDestination != FVector::ZeroVector)
	{
		UEnemyLogManager::EnemyLog(EEnemyLogType::SkeletonMage, FString::Printf(TEXT("[스켈레톤 메이지] 텔레포트 이동: %.2f"), 
			FVector::Dist(GetActorLocation(), TeleportDestination)));
		
		//SpawnTeleportEffectAtLocation(GetActorLocation()); // 현재 위치에 이펙트 생성
		
		// --- 수정된 부분: 목적지 좌표에서 Z축으로 10만큼 아래로 이동 ---
		FVector AdjustedDestination = TeleportDestination;
		AdjustedDestination.Z -= 40.0f; 
		
		SetActorLocation(AdjustedDestination); // 조정된 위치로 텔레포트 이동
		// --------------------------------------------------------

		// 이펙트는 원래 위치나 필요에 따라 AdjustedDestination 사용
		if ( TeleportOutEffect ) SpawnTeleportEffectAtLocation(TeleportDestination, TeleportOutEffect); 
		
		// TargetCharacter를 찾아서 바라보도록 회전
		if (TargetCharacter)
		{
			const FVector TargetLocation = TargetCharacter->GetActorLocation();
			// 현재 위치에서 타겟 위치를 바라보는 회전값 계산
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);

			// Z축(Yaw) 회전만 적용하여 수평으로 바라보게 함
			SetActorRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
		}
		
	}
}

void ABossSkeletonMage::SpawnTeleportEffectAtLocation(const FVector& Location, class UNiagaraSystem* EffectToSpawn)
{
	if (TeleportOutEffect == nullptr) return;

	FVector SpawnLocation = Location;

	// 바닥을 찾기 위해 라인 트레이스를 수행합니다.
	FHitResult HitResult;
	const FVector StartTrace = Location + FVector(0.f, 0.f, 100.f); // 약간 위에서 시작
	const FVector EndTrace = Location - FVector(0.f, 0.f, 1000.f); // 아래로 1000 유닛
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	// 라인 트레이스로 바닥 위치를 찾습니다.
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, TraceParams))
	{
		// 충돌 지점을 스폰 위치로 설정합니다.
		SpawnLocation = HitResult.Location;
	}

	// 나이아가라 이펙트를 스폰합니다.
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EffectToSpawn, SpawnLocation,
		FRotator::ZeroRotator, FVector(1.f));
}

void ABossSkeletonMage::PlayFireBallMontage()
{
	if ( BlackboardComp == nullptr ) return;
	
	BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.FireBallDelay); // 행동 딜레이 설정
	
	if ( FireBallMontage )
	{
		PlayAnimMontage(FireBallMontage);
	}
}

// 애님 노티파이에서 호출할 함수
void ABossSkeletonMage::ShootFireBall()
{
	// 필요한 모든 컴포넌트와 변수가 유효한지 확인합니다.
	if (!IsValid(TargetCharacter) || !IsValid(FireBallSpawnPoint) || FireBallClass == nullptr)
	{
		return;
	}

	// FireBallSpawnPoint의 월드 위치와 회전값을 가져옵니다.
	const FVector SpawnLocation = FireBallSpawnPoint->GetComponentLocation();

	// 발사 위치에서 타겟을 향하는 방향을 계산합니다.
	const FVector TargetLocation = TargetCharacter->GetActorLocation();
	const FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);

	// 스폰 파라미터를 설정합니다.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 월드에 발사체를 스폰합니다.
	GetWorld()->SpawnActor<ABaseEnemyProjectile>(FireBallClass, SpawnLocation, SpawnRotation, SpawnParams);
}

void ABossSkeletonMage::StartSummoning(const FVector& Location1, const FVector& Location2)
{
	if ( BlackboardComp == nullptr ) return;
	
	SpawnSummonEffectAtLocation(GetActorLocation()); // 마법사가 소환 시작 이펙트
	
	BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.SummonEnemyDelay); // 행동 딜레이 설정
	
	SummonLocations.Empty();
	SummonLocations.Add(Location1);
	SummonLocations.Add(Location2);
	
	if ( SummonEnemyMontage )
	{
		PlayAnimMontage(SummonEnemyMontage);
	}
}

// 애님 노티파이에서 호출할 함수
void ABossSkeletonMage::SummonEnemy()
{
	//UE_LOG(LogTemp, Warning, TEXT("SummonEnemy called") );
	// 소환할 적 클래스가 없거나 소환 위치가 없으면 함수를 종료합니다.
	if (SummonableEnemyClasses.Num() == 0 || SummonLocations.Num() == 0) return;
	
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	
	// 저장된 각 소환 위치에 대해 반복합니다.
	for (const FVector& SpawnLocation : SummonLocations)
	{
		// 소환할 적 클래스 배열에서 무작위 인덱스를 선택합니다.
		const int32 RandomIndex = FMath::RandRange(0, SummonableEnemyClasses.Num() - 1);
		TSubclassOf<ABaseEnemy> EnemyClassToSummon = SummonableEnemyClasses[RandomIndex];

		if (EnemyClassToSummon)
		{
			// 스폰 위치에서 타겟을 바라보는 회전값을 계산합니다.
			const FRotator LookAtRotation =
				UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetCharacter->GetActorLocation());
			// 수평으로만 바라보도록 Yaw 값만 사용합니다.
			const FRotator SpawnRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);

			// 적의 캡슐 컴포넌트 높이만큼 Z 오프셋을 적용하여 땅에 정확히 스폰되도록 합니다.
			float SafeZOffset = 0.f;
			ACharacter* EnemyCDO = Cast<ACharacter>(EnemyClassToSummon->GetDefaultObject());
			if (EnemyCDO && EnemyCDO->GetCapsuleComponent())
			{
				SafeZOffset = EnemyCDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			}
			
			ABaseEnemy* SpawnedEnemy = 
				World->SpawnActor<ABaseEnemy>(
				EnemyClassToSummon, 
				SpawnLocation + FVector(0.f, 0.f, SafeZOffset), 
				SpawnRotation, 
				SpawnParams);

			if ( SummonEffectFromEnemy )
			{
				// 나이아가라 이펙트를 스폰합니다.
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SummonEffectFromEnemy,
					SpawnLocation + FVector(0.f, 0.f, 2.f),
					FRotator::ZeroRotator, FVector(20.f));
			}
			
			// 보스와 소환되는 거리 로그 매니저로 출력
			UEnemyLogManager::EnemyLog(EEnemyLogType::SkeletonMage, FString::Printf(TEXT("[스켈레톤 메이지] 적 소환: %s (거리: %.2f)"), 
				*SpawnedEnemy->GetName(), FVector::Dist(GetActorLocation(), SpawnLocation)));
			
		}
	}
}

void ABossSkeletonMage::SpawnSummonEffectAtLocation(const FVector& Location)
{
	if (SummonEffectFromMage == nullptr) return;

	FVector SpawnLocation = Location;

	// 바닥을 찾기 위해 라인 트레이스를 수행합니다.
	FHitResult HitResult;
	const FVector StartTrace = Location + FVector(0.f, 0.f, 100.f); // 약간 위에서 시작
	const FVector EndTrace = Location - FVector(0.f, 0.f, 1000.f); // 아래로 1000 유닛
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	// 라인 트레이스로 바닥 위치를 찾습니다.
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, TraceParams))
	{
		// 충돌 지점보다 약간 위에 스폰 위치를 설정합니다.
		SpawnLocation = HitResult.Location + FVector(0.f, 0.f, 5.f);
	}

	// 나이아가라 이펙트를 스폰합니다.
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SummonEffectFromMage, SpawnLocation,
		FRotator::ZeroRotator, FVector(10.f));
}

void ABossSkeletonMage::PlayGroundAreaAttackMontage()
{
	if ( BlackboardComp == nullptr ) return;
	
	BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.GroundAttackDelay); // 행동 딜레이 설정

	GroundTargetingComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		GroundTargetingEffect,
		TargetCharacter->GetActorLocation(), // 초기 위치
		FRotator::ZeroRotator,
		FVector(1.f),
		true
	);
	
	if ( GroundAreaAttackMontage )
	{
		PlayAnimMontage(GroundAreaAttackMontage);
	}
}

void ABossSkeletonMage::TraceTargetCharacterForGroundAttackEffect(float DeltaTime)
{
	// 그라운드 타겟팅 이펙트 위치 업데이트
	if (GroundTargetingComponent && GroundTargetingComponent->IsValidLowLevel() && IsValid(TargetCharacter))
	{
		FVector CharacterLocation = TargetCharacter->GetActorLocation();
		FVector TargetLocation = CharacterLocation;

		// 캐릭터 위치에서 아래로 라인 트레이스를 실행하여 바닥을 찾습니다.
		FHitResult HitResult;
		FVector StartTrace = CharacterLocation;
		FVector EndTrace = CharacterLocation - FVector(0.f, 0.f, 1000.f); // 아래로 1000 유닛
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		TraceParams.AddIgnoredActor(TargetCharacter);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, TraceParams))
		{
			// 충돌 지점을 타겟 위치로 설정합니다.
			TargetLocation = HitResult.Location;
		}

		// 이펙트의 월드 위치를 업데이트합니다. (Z축으로 2만큼 상승)
		GroundTargetingComponent->SetWorldLocation(TargetLocation + FVector(0.f, 0.f, 5.f));
	}
}

// 애님 노티파이에서 호출할 함수
void ABossSkeletonMage::GroundAreaAttack()
{
	if (!IsValid(TargetCharacter) || GroundAttackClass == nullptr) return;

	// 타겟팅 이펙트가 존재하면 제거합니다.
	if (GroundTargetingComponent && GroundTargetingComponent->IsValidLowLevel())
	{
		GroundTargetingComponent->DestroyComponent();
		GroundTargetingComponent = nullptr;
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("a"));

	const FVector CharacterLocation = TargetCharacter->GetActorLocation();
	FVector SpawnLocation = CharacterLocation; // 기본 스폰 위치

	// 바닥을 찾기 위해 라인 트레이스를 수행합니다.
	FHitResult HitResult;
	const FVector StartTrace = CharacterLocation;
	const FVector EndTrace = CharacterLocation - FVector(0.f, 0.f, 1000.f); // 아래로 1000 유닛
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(TargetCharacter);
	TraceParams.AddIgnoredActor(this);

	// 라인 트레이스로 바닥 위치를 찾습니다.
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_WorldStatic, TraceParams))
	{
		// 충돌 지점을 스폰 위치로 설정합니다.
		SpawnLocation = HitResult.Location;
	}
	// 장판은 보통 바닥에 평평하게 놓이므로 회전은 필요 없습니다.
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	// 항상 스폰되도록 설정합니다.
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 월드에 장판 프로젝타일을 스폰합니다.
	AGroundAttackProjectile* SpawnedProjectile = GetWorld()->SpawnActor<AGroundAttackProjectile>(GroundAttackClass, SpawnLocation, SpawnRotation, SpawnParams);

	// 스폰된 액터가 유효하고, 유지 시간이 0보다 크면 LifeSpan을 설정합니다.
	if (SpawnedProjectile)
	{
		//SpawnedProjectile->SetLifeSpan(GroundAttackDuration);
		SpawnedProjectile->DurationTime = AttackStruct.GroundAttackDuration; // 장판 유지 시간 설정
	}
	if ( GroundAttackEffect ) 
	{
		// 나이아가라 이펙트를 스폰합니다.
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GroundAttackEffect,
			SpawnLocation + FVector(0.f, 0.f, 2.f),
			FRotator::ZeroRotator, FVector(5.f));
	}
}

void ABossSkeletonMage::PlayPushTargetMontage()
{
	if ( BlackboardComp == nullptr ) return;
	
	BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.PushTargetDelay); // 행동 딜레이 설정
	
	if ( PushTargetMontage )
	{
		PlayAnimMontage(PushTargetMontage);
	}
}

void ABossSkeletonMage::CreateMagicShield()
{
	if ( MagicShieldEffect )
	{
		// 나이아가라 이펙트를 스폰합니다.
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MagicShieldEffect,
			GetActorLocation() + FVector(0.f, 0.f, 10.f),
			FRotator::ZeroRotator, FVector(1.f));
	}
}

void ABossSkeletonMage::GravityAttack()
{
	if ( BlackboardComp == nullptr ) return;
	
	BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.GravityAttackDelay);
	
	if ( GravityAttackMontage )
	{
		PlayAnimMontage(GravityAttackMontage);
	}
}

void ABossSkeletonMage::StartGravityAttack()
{
	if (!TargetCharacter || !GravityGroundEffect) return;

	UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		DefaultGravityScale = MoveComp->GravityScale;
		DefaultAirControl = MoveComp->AirControl;
		DefaultMaxWalkSpeed = MoveComp->MaxWalkSpeed;
	}
	
	// 바닥 위치 계산 로직 (기존과 동일)
	const FVector CharacterLocation = TargetCharacter->GetActorLocation();
	FVector SpawnLocation = CharacterLocation;
	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(TargetCharacter);
	TraceParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, CharacterLocation, CharacterLocation - FVector(0.f, 0.f, 1000.f), ECC_WorldStatic, TraceParams))
	{
		SpawnLocation = HitResult.Location;
	}

	// 상태값 설정
	GravityAttackCenter = SpawnLocation + FVector(0.f, 0.f, GravityHalfHeight - 200.f);
	GravityTimer = 0.0f; // 타이머 초기화

	// 이펙트 스폰
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GravityGroundEffect, SpawnLocation + FVector(0.f, 0.f, 2.f), FRotator::ZeroRotator, FVector(1.f));

	// 시전 시간 뒤에 실제 캡슐 판정 활성화
	FTimerHandle GravityActivationTimer;
	GetWorldTimerManager().SetTimer(GravityActivationTimer, [this]()
	{
		bIsGravityAttackActive = true; 
	}, 1.0f, false); // 1.0초 지연 (원하는 시간으로 조절 가능)
	
	FTimerHandle ImpactEffectTimer;
	// 람다 캡처에 SpawnLocation을 추가하여 시작 시점의 바닥 위치를 기억하게 합니다.
	GetWorldTimerManager().SetTimer(ImpactEffectTimer, [this, SpawnLocation]()
	{
		if (GravityImpactEffect)
		{
			// 시작 시 장판(GravityGroundEffect) 위치와 동일한 위치에서 Z축으로 -100만큼 조정
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), GravityImpactEffect,
				SpawnLocation + FVector(0.f, 0.f, -100.f), 
				FRotator::ZeroRotator, FVector(1.f));
		}
	}, 4.7f, false);
	
	// 일정 시간 뒤 종료 예약
	FTimerHandle GravityEndTimer;
	GetWorldTimerManager().SetTimer(GravityEndTimer, this, &ABossSkeletonMage::EndGravityAttack, GravityDuration, false);
}

void ABossSkeletonMage::EndGravityAttack()
{
	bIsGravityAttackActive = false;
	
	// 1. 범위 내의 모든 캐릭터를 찾기 위한 설정
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape GravityCapsule = FCollisionShape::MakeCapsule(GravityRadius, GravityHalfHeight);
	FCollisionQueryParams OverlapParams;
	OverlapParams.AddIgnoredActor(this);

	// 2. 공격 종료 시 캡슐 범위 내에 있는 캐릭터들 체크
	if (GetWorld()->OverlapMultiByChannel(OverlapResults, GravityAttackCenter, FQuat::Identity, ECC_Pawn, GravityCapsule, OverlapParams))
	{
		for (auto& Result : OverlapResults)
		{
			ACharacter* OverlappedChar = Cast<ACharacter>(Result.GetActor());
			if (OverlappedChar && OverlappedChar->ActorHasTag(TEXT("Player")))
			{
				UCharacterMovementComponent* MoveComp = OverlappedChar->GetCharacterMovement();
				if (MoveComp)
				{
					// 1. 원래 상태로 복구
					MoveComp->GravityScale = DefaultGravityScale;
					MoveComp->AirControl = DefaultAirControl;
					MoveComp->MaxWalkSpeed = DefaultMaxWalkSpeed;

					// 2. 바닥으로 빠르게 발사 (Z축 하향 속도 부여)
					// -1500.f 정도면 매우 빠르게 바닥으로 내리꽂힙니다.
					FVector SlamVelocity = FVector(0.f, 0.f, -4000.f); 
					OverlappedChar->LaunchCharacter(SlamVelocity, false, true);
                
					// 대미지 주기
					UGameplayStatics::ApplyDamage(OverlappedChar, 
						AttackStruct.GravityAttackDamage, GetController(), this, UDamageType::StaticClass());
					
					UEnemyLogManager::EnemyLog(EEnemyLogType::SkeletonMage, 
						FString::Printf(TEXT("[스켈레톤 메이지] 중력 공격으로 플레이어에게 %.2f 대미지"), AttackStruct.GravityAttackDamage));
				}
			}
		}
	}
	
	// 3. (혹시 범위 밖에 나갔더라도) 타겟팅된 메인 플레이어는 확실하게 복구
	if (TargetCharacter)
	{
		UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement();
		if (MoveComp && MoveComp->GravityScale < 1.0f)
		{
			MoveComp->GravityScale = DefaultGravityScale;
			MoveComp->AirControl = DefaultAirControl;
			MoveComp->MaxWalkSpeed = DefaultMaxWalkSpeed;
		}
	}
}

void ABossSkeletonMage::HandleGravityAttack(float DeltaTime)
{
	if (bIsGravityAttackActive)
	{
		// 디버그 캡슐 그리기는 매 프레임 실행 (시각화 유지용)
		DrawDebugCapsule(GetWorld(), GravityAttackCenter, GravityHalfHeight, GravityRadius, 
			FQuat::Identity, FColor::Purple, false, DeltaTime * 2.f, 0, 1.0f);

		// 범위 체크
		TArray<FOverlapResult> OverlapResults;
		FCollisionShape GravityCapsule = FCollisionShape::MakeCapsule(GravityRadius, GravityHalfHeight);
		FCollisionQueryParams OverlapParams;
		OverlapParams.AddIgnoredActor(this);

		bool bIsPlayerInArea = false;
		if (GetWorld()->OverlapMultiByChannel(OverlapResults, GravityAttackCenter, FQuat::Identity, ECC_Pawn, GravityCapsule, OverlapParams))
		{
			for (auto& Result : OverlapResults)
			{
				ACharacter* OverlappedChar = Cast<ACharacter>(Result.GetActor());
				if (OverlappedChar && OverlappedChar->ActorHasTag(TEXT("Player")))
				{
					bIsPlayerInArea = true;
					UCharacterMovementComponent* MoveComp = OverlappedChar->GetCharacterMovement();
					if (MoveComp)
					{
						// 영역 안에 있는 동안 지속적으로 무중력 적용
						MoveComp->GravityScale = 0.05f; 
						MoveComp->AirControl = 0.7f;  // 공중 제어력 증가
						MoveComp->MaxWalkSpeed = 200.f; // 좀 느리게
						
						// Z축으로 살짝 뜨게 하는 힘 (둥둥 뜨는 느낌)
						if (OverlappedChar->GetVelocity().Z < 100.f)
						{
							MoveComp->AddImpulse(FVector(0.f, 0.f, 20.f), true);
						}
					}
				}
			}
		}

		// 만약 플레이어가 영역 밖으로 나갔다면 중력 원복
		if (!bIsPlayerInArea && TargetCharacter)
		{
			UCharacterMovementComponent* MoveComp = TargetCharacter->GetCharacterMovement();
			if (MoveComp && MoveComp->GravityScale < 1.0f)
			{
				MoveComp->GravityScale = DefaultGravityScale;
				MoveComp->AirControl = DefaultAirControl;
				MoveComp->MaxWalkSpeed = DefaultMaxWalkSpeed;
			}
		}
	}
}

