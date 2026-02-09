#include "EnemySpawner/EnemySpawner.h"

#include "BaseEnemy.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; 
	
	RootCollisionSphere = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootCollisionSphere"));
	RootComponent = RootCollisionSphere;

	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
	SpawnerMesh->SetupAttachment(RootComponent);
	SpawnerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent);

	PlayerDetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectSphere"));
	PlayerDetectSphere->SetupAttachment(RootComponent);
	PlayerDetectSphere->SetHiddenInGame(false);
	PlayerDetectSphere->SetSphereRadius(1000.f); // 기본값
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if ( PlayerDetectSphere )
	{
		PlayerDetectSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemySpawner::OnBeginOverlapPlayerDetectSphere);
		PlayerDetectSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemySpawner::OnEndOverlapPlayerDetectSphere);
	}
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit();

	// 1. 쿨타임 감소
	if (CurrentSpawnCooldown > 0.0f)
	{
		CurrentSpawnCooldown -= DeltaTime;
	}

	// 2. 쿨타임이 끝났고, 플레이어가 유효하다면 시야 체크 시도
	if (CurrentSpawnCooldown <= 0.0f && TargetCharacter)
	{
		// 시야 체크 (Line Trace)
		if (CanSeePlayer())
		{
			SpawnEnemy();
            
			// 스폰 후 쿨타임 재설정
			CurrentSpawnCooldown = SpawnDelay;
		}
		else
		{
			// 시야가 안 보이면?
			// 매 프레임 검사하면 너무 무거우니 0.2초 정도 있다가 다시 검사하게 설정 (최적화 팁)
			CurrentSpawnCooldown = 0.2f; 
		}
	}
}

void AEnemySpawner::PollInit()
{
	if ( bTargetInitalize == false )
	{
		TargetCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0); // 월드에서 첫 번째 플레이어 캐릭터를 가져옵니다.
		if ( TargetCharacter ) // 캐릭터가 유효한지 확인합니다.
		{
			bTargetInitalize = true; // 캐릭터가 유효하면 초기화 플래그를 true로 설정합니다.
		}
	}
}

// 플레이어가 감지 범위에 들어왔을 때
void AEnemySpawner::OnBeginOverlapPlayerDetectSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 플레이어 태그 확인
	if ( OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")) )
	{
		if (!TargetCharacter)
		{
			TargetCharacter = Cast<ACharacter>(OtherActor);
		}

		// 틱을 켜서 실시간 감시 시작
		SetActorTickEnabled(true);
        
		// 들어오자마자 바로 쏠 수 있게 쿨타임 초기화 (원한다면)
		CurrentSpawnCooldown = 0.0f; 
	}
}

// 플레이어가 감지 범위에서 나갔을 때
void AEnemySpawner::OnEndOverlapPlayerDetectSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if ( OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")) )
	{
		// 플레이어가 멀어지면 더 이상 검사할 필요 없음 -> 틱 끄기 (성능 절약)
		SetActorTickEnabled(false);
	}
}

bool AEnemySpawner::CanSeePlayer() const
{
	// 타겟이 없으면 보이지 않는 것으로 간주
	if (!TargetCharacter) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	// 시작점: 스포너 위치 (바닥보단 살짝 위)
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	// 끝점: 타겟 플레이어 위치
	FVector End = TargetCharacter->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);            // 스포너 자신은 무시
	CollisionParams.AddIgnoredActor(TargetCharacter); // 플레이어 자체는 무시 (플레이어 '앞'을 가리는 벽만 체크)

	// Visibility 채널(시야)로 라인 트레이스 수행
	bool bHit = World->LineTraceSingleByChannel(
	   HitResult,
	   Start,
	   End,
	   ECollisionChannel::ECC_Visibility,
	   CollisionParams
	);

	// 무언가(벽)에 맞았다면 시야가 막힌 것이므로 false, 안 맞았으면 트인 것이므로 true
	return !bHit;
}

void AEnemySpawner::SpawnEnemy()
{
	// 0. 시야 체크: 플레이어가 보이지 않으면 스폰하지 않음
	if (CanSeePlayer() == false) return;
	
	if (SpawningEnemyClasses.Num() > 0 && SpawnLocation)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// 배열에서 랜덤하게 하나 선택
			int32 RandomIndex = FMath::RandRange(0, SpawningEnemyClasses.Num() - 1);
			TSubclassOf<ABaseEnemy> SelectedEnemyClass = SpawningEnemyClasses[RandomIndex];

			// 선택된 클래스가 유효한지 확인
			if (!SelectedEnemyClass) return;

			// 기본 스폰 위치
			FVector OriginLocation = SpawnLocation->GetComponentLocation();
			FVector FinalSpawnLocation = OriginLocation;
			FRotator SpawnRotation = SpawnLocation->GetComponentRotation();

			// 1. 내비게이션 시스템을 통해 랜덤 위치 찾기
			UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
			float RandomRadius = 100.0f; 

			if (NavSystem)
			{
				FNavLocation RandomPoint;
				// OriginLocation 기준 RandomRadius 반경 내의 랜덤한 이동 가능 지점 찾기
				if (NavSystem->GetRandomReachablePointInRadius(OriginLocation, RandomRadius, RandomPoint))
				{
					FinalSpawnLocation = RandomPoint.Location;
				}
				
				// 2. 캡슐 컴포넌트의 높이만큼 Z축 보정 (땅에 파묻히지 않게)
				float SafeZOffset = 0.f;
				ACharacter* EnemyCDO = Cast<ACharacter>(SelectedEnemyClass->GetDefaultObject());
				if (EnemyCDO && EnemyCDO->GetCapsuleComponent())
				{
					SafeZOffset = EnemyCDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				}

				// 위치 보정 적용
				FinalSpawnLocation.Z += SafeZOffset;

				// 3. 스폰 파라미터 설정 및 스폰
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				World->SpawnActor<ABaseEnemy>(
					SelectedEnemyClass,
					FinalSpawnLocation,
					SpawnRotation,
					SpawnParams
				);
			}
		}
	}
}

#if WITH_EDITOR
void AEnemySpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AEnemySpawner, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( PlayerDetectSphere ) PlayerDetectSphere->SetVisibility(true);
		}
		else
		{
			if ( PlayerDetectSphere ) PlayerDetectSphere->SetVisibility(false);
		}
	}
	
}
#endif
