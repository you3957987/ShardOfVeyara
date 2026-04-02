#include "EnemyBoss/BaseBossEnemy.h"

#include "AIController.h"
#include "EnemyLogManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ProgressBar.h"
#include "Components/sphereComponent.h"
#include "Components/WidgetComponent.h"
#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"
#include "EnemyHUD/EnemyHealthBarWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h" 
#include "Interface/PlayerDeadInterface.h"

ABaseBossEnemy::ABaseBossEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터 메시의 콜리전을 비활성화합니다.
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	// 캡슐 컴포넌트가 카메라에 반응하지 않도록 설정합니다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// 체력 바 위젯 컴포넌트 생성 및 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World); 

	PlayerDetectRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectRangeSphere"));
	PlayerDetectRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트
	PlayerDetectRangeSphere->ShapeColor = FColor::Green;
	PlayerDetectRangeSphere->SetSphereRadius(PlayerDetectRange);
	PlayerDetectRangeSphere->SetVisibility(false);
	PlayerDetectRangeSphere->SetHiddenInGame(false); 
	
	Tags.Add(FName("Enemy")); // 적 캐릭터 태그 추가 -> 이걸 이용해서 프로젝트에서 플러그인 에너미 접근. 매우 중요!!!!
	Tags.Add(FName("Boss")); // 보스 태그 추가
	
	// AI 컨트롤러가 자동 빙의 하는거 제한. 범위 안에 플레이어가 들어왔을 때 오버랩 이벤트로 빙의
	AutoPossessAI = EAutoPossessAI::Disabled;

	// 오리엔트 투 무브먼트 비활성화 == 
	GetCharacterMovement()->bOrientRotationToMovement = false;
	// 컨트롤러 선호 회전 설정
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
	// 모션 워핑 
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void ABaseBossEnemy::BeginPlay()
{
	Super::BeginPlay();

	if ( HealthBarWidget ) // 체력바 위젯에서 프로그레스바 설정
	{
		UEnemyHealthBarWidget* HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if ( HealthBar )
		{
			HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
		}
	}

	if ( PlayerDetectRangeSphere )
	{
		// 오버랩 이벤트 바인딩
		PlayerDetectRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseBossEnemy::OnPlayerDetectOverlapBegin);
	}
	
	TestDeadLogic();
}

void ABaseBossEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit(DeltaTime);
	UpdateHealthBarWidget(DeltaTime);
}

void ABaseBossEnemy::PollInit(float DeltaTime)
{
	if ( bSetBlackboard == false )
	{
		AAIController* AIController = Cast<AAIController>(GetController());
		if (AIController && AIController->GetBlackboardComponent())
		{
			BlackboardComp = AIController->GetBlackboardComponent();
			if ( BlackboardComp != nullptr )
			{
				bSetBlackboard = true;
			}
		}
	}
	if ( bTargetInitalize == false )
	{
		TargetCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0); // 월드에서 첫 번째 플레이어 캐릭터를 가져옵니다.
		if ( TargetCharacter ) // 캐릭터가 유효한지 확인합니다.
		{
			IPlayerDeadInterface* DeadInterface = Cast<IPlayerDeadInterface>(TargetCharacter);
			if (DeadInterface)
			{
				// 이전에 바인딩된 게 있다면 제거하고 등록 (중복 방지 안전장치)
				DeadInterface->GetOnPlayerDeadDelegate().RemoveDynamic(this, &ABaseBossEnemy::PlayerDeadLog);
				DeadInterface->GetOnPlayerDeadDelegate().AddDynamic(this, &ABaseBossEnemy::PlayerDeadLog);
            
				UE_LOG(LogTemp, Error, TEXT("asdasdsad"));
				
				bTargetInitalize = true; 
			}
			else 
			{
				UE_LOG(LogTemp, Error, TEXT("Player does not implement IPlayerDeadInterface"));
			}
		}
	}
}

void ABaseBossEnemy::OnPlayerDetectOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 오버랩된 액터가 플레이어인지 확인합니다.
	if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
	{
		SpawnDefaultController();// 스폰 몽타주 사용 안하면 자동 빙의 설정
		PlayerDetectRangeSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 한 번 감지되면 비활성화
		
		StartBattleLog(); // 전투 로그 시작
	}
}

void ABaseBossEnemy::UpdateHealthBarWidget(float DeltaTime)
{
	if (HealthBarWidget && HealthBarWidget->IsVisible())
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC && PC->PlayerCameraManager)
		{
			const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
			const FVector WidgetLocation = HealthBarWidget->GetComponentLocation();

			// 위젯에서 카메라를 바라보는 방향의 회전값을 계산합니다.
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);

			// 위젯이 항상 수평을 유지하도록 Yaw 값만 사용하여 회전을 설정합니다.
			HealthBarWidget->SetWorldRotation(FRotator(0.f, LookAtRotation.Yaw, 0.f));
		}

		UEnemyHealthBarWidget* HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if ( HealthBar )
		{
			HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
		}
	}
}

float ABaseBossEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Warning, TEXT("Boss Take Damage : %f"), DamageToApply);

	// 메쉬 이름 가져오기 헬퍼
	FString MeshName = GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? 
		GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");
	
	if ( DamageToApply > 0.f )
	{
		Health -= DamageToApply;
		
		UEnemyLogManager::EnemyLog( GetLogTypeFromEnemyType(), 
		FString::Printf(TEXT("[%s]가 [%.f] 대미지 받음 (%.f / %.f)"), 
			*MeshName, DamageToApply, MaxHealth, Health));
		
		if ( Health <= 0.f )
		{
			Die();
		}
		UEnemyHealthBarWidget* HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if ( HealthBar )
		{
			HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
		}
	}
	
	return DamageToApply;
}

void ABaseBossEnemy::Die()
{
	if ( DeathMontage ) PlayAnimMontage(DeathMontage);
	
	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	// AI 로직 중지
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsBool(TEXT("IsDead"), true);
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
	
	EndBattleLog(); // 전투 로그 종료
}

void ABaseBossEnemy::AfterDieMontageEnd()
{
	if ( GetMesh() )
	{
		GetMesh()->bPauseAnims = true;
	}
	
	// 0.3초 후에 SpawnEffectAndDestroy 함수를 호출합니다.
	GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, this,
		&ABossSkeletonMage::SpawnDeadEffectAndDestroy, 1.0f, false);
}

void ABaseBossEnemy::SpawnDeadEffectAndDestroy()
{
	if ( DeathEffectCascade )
	{
		const FVector SpawnLocation = GetMesh()->GetComponentLocation() + (GetActorForwardVector() * DeathEffectForwardOffset);
		const FRotator SpawnRotation = GetActorRotation();
		const FVector SpawnScale = FVector(DeathEffectScale);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathEffectCascade,
		 SpawnLocation, SpawnRotation, SpawnScale);
	}

	DropItemsAfterDead(); // 아이템 드롭
	
	Destroy(); // 이펙트가 없으면 바로 액터 삭제
}

void ABaseBossEnemy::StartFocusPlayerAfterAttack()
{
	bFocusPlayerAfterAttack = true; // 공격 후 포커스 시작 플래그를 true로 설정
}

void ABaseBossEnemy::DropItemsAfterDead()
{
	for (const TSubclassOf<AActor>& ItemClassToSpawn : DropItems)
	{
		if ( !ItemClassToSpawn ) continue;

		// 적의 현재 위치 (발 밑)
		FVector SpawnLocation = GetActorLocation();

		// 캡슐의 절반 높이만큼 올려서 아이템이 땅에 닿도록 조정
		AActor* ItemCDO = ItemClassToSpawn->GetDefaultObject<AActor>();
		if ( ItemCDO )
		{
			UCapsuleComponent* ItemCapsule = ItemCDO->FindComponentByClass<UCapsuleComponent>();
			if ( ItemCapsule )
			{
				SpawnLocation.Z += ItemCapsule->GetScaledCapsuleHalfHeight();
			}
		}

		// 여러 아이템이 완전히 겹치지 않도록 X, Y 주변에 약간의 랜덤 오프셋 주기
		const float RandomXY = 40.f;
		SpawnLocation.X += FMath::RandRange(-RandomXY, RandomXY);
		SpawnLocation.Y += FMath::RandRange(-RandomXY, RandomXY);

		// 회전도 랜덤하게 설정
		FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);

		// 월드에 아이템 액터 스폰
		GetWorld()->SpawnActor<AActor>(ItemClassToSpawn, SpawnLocation, SpawnRotation);
	}
}

// 공격 전에 한번씩 호출
void ABaseBossEnemy::UpdateMotionWarpTarget()
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
		
		if (bDebugMode == true)
		{
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
}

void ABaseBossEnemy::UpdateMotionWarpTargetToFront()
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

		if (bDebugMode == true)
		{
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
}


void ABaseBossEnemy::TestDeadLogic()
{
	// 죽음 로직 체크
	if (bCheckDeadLogic)
	{
		FTimerHandle DeadTestTimerHandle;
		// 5초 후에 체력을 0으로 만들고 Die() 함수를 호출합니다.
		GetWorld()->GetTimerManager().SetTimer(DeadTestTimerHandle, [this]()
		{
		 Health = 0.f;
		 Die();
		}, 5.0f, false);
	}
}

void ABaseBossEnemy::StartBattleLog()
{
	// 이미 전투 중이면 중복 기록 안 함
	if (bIsInBattle) return;

	bIsInBattle = true;
	BattleStartTime = GetWorld()->GetTimeSeconds(); // 현재 월드 시간 초단위 저장

	// 메쉬 이름 가져오기 헬퍼
	FString MeshName = GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? 
		GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");

	UEnemyLogManager::EnemyLog(GetLogTypeFromEnemyType(), 
		FString::Printf(TEXT("[%s] 전투 진입 (시작 시간: %.2f)"), *MeshName, BattleStartTime));
}

void ABaseBossEnemy::EndBattleLog()
{
	// 전투 중이 아니면 종료 로그 안 함
	if (!bIsInBattle) return;

	float EndTime = GetWorld()->GetTimeSeconds();
	float BattleDuration = EndTime - BattleStartTime; // 지속 시간 계산

	FString MeshName = GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? 
		GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");

	UEnemyLogManager::EnemyLog(GetLogTypeFromEnemyType(), 
		FString::Printf(TEXT("[%s] 전투 종료 | 소요 시간: %.2f초 (시작: %.2f / 종료: %.2f)"), 
			*MeshName, BattleDuration, BattleStartTime, EndTime));

	// 상태 초기화
	bIsInBattle = false;
	BattleStartTime = 0.f;
}

void ABaseBossEnemy::PlayerDeadLog()
{
	// 메쉬 이름 가져오기 헬퍼
	FString MeshName = GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? 
		GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");
	
	UEnemyLogManager::EnemyLog(GetLogTypeFromEnemyType(), 
		FString::Printf(TEXT("보스 [%s] 의 공격으로 플레이어 사망"), *MeshName));
	
	if ( BlackboardComp )
	{
		BlackboardComp->SetValueAsBool(TEXT("IsDead"), true);
	} 
	
	EndBattleLog();
}

void ABaseBossEnemy::AttackPatternLog(FString PatternName) const
{
	// 메쉬 이름 가져오기
	FString MeshName = GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? 
		GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");

	// 로그 기록: [거리분류] 패턴명 선택
	UEnemyLogManager::EnemyLog(GetLogTypeFromEnemyType(), 
		FString::Printf(TEXT("[%s] 패턴 결정 | 거리: [%s] | 패턴: [%s] "), 
			*MeshName, *SelectedRangeName ,*PatternName));
}

EEnemyLogType ABaseBossEnemy::GetLogTypeFromEnemyType() const
{
	FString ActorName = GetName();

	// 1. 클래스 이름 또는 액터 이름으로 보스 종류 판별
	if (ActorName.Contains(TEXT("SkeletonMage")))
	{
		return EEnemyLogType::SkeletonMage;
	}
	else if (ActorName.Contains(TEXT("BlackKnight")))
	{
		return EEnemyLogType::BlackKnight;
	}
	else if (ActorName.Contains(TEXT("Worm")))
	{
		return EEnemyLogType::Worm;
	}
	else if (ActorName.Contains(TEXT("MagicSwordMan")))
	{
		return EEnemyLogType::MagicSwordMan;
	}

	// 기본값 (판별 불가능할 경우)
	return EEnemyLogType::SkeletonMage; 
}

#if	WITH_EDITOR
void ABaseBossEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseBossEnemy, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( PlayerDetectRangeSphere ) PlayerDetectRangeSphere->SetVisibility(true);
		}
		else
		{
			if ( PlayerDetectRangeSphere ) PlayerDetectRangeSphere->SetVisibility(false);
		}
	}
	
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseBossEnemy, MoveSpeed) )
	{
		if ( GetCharacterMovement() )
		{
			GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseBossEnemy, PlayerDetectRange) )
	{
		if ( PlayerDetectRangeSphere )
		{
			// 플레이어 인식 범위 스피어의 반지름을 PlayerDetectRange 값으로 설정합니다.
			PlayerDetectRangeSphere->SetSphereRadius(PlayerDetectRange);
		}
	}
}
#endif