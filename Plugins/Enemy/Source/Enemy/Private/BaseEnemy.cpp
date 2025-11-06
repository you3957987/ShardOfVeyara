#include "BaseEnemy.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/Progressbar.h"
#include "Kismet/GameplayStatics.h" // UGameplayStatics 사용을 위한 헤더 파일
#include "Components/SphereComponent.h" // USphereComponent 사용을 위한 헤더 파일
#include "Components/WidgetComponent.h"
#include "EnemyHUD/EnemyHealthBarWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyProjectile/BaseEnemyProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트

	DetectRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AcceptRangeSphere"));
	DetectRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트

	ChaseRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ChaseRangeSphere"));
	ChaseRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트

	MeleeAttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttackPoint"));
	MeleeAttackPoint->SetupAttachment(RootComponent); // 루트 컴포넌트
	
	AttackRangePointSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangePointSphere"));
	AttackRangePointSphere->SetupAttachment(MeleeAttackPoint); // AttackPoint에 부착

	RangedAttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RangedAttackPoint"));
	RangedAttackPoint->SetupAttachment(RootComponent); // 루트 컴포넌트

	// 체력 바 위젯 컴포넌트 생성 및 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World); // 월드 공간으로 변경
	
	// 적 캐릭터 태그 추가 -> 이걸 이용해서 프로젝트에서 플러그인 에너미 접근. 매우 중요!!!!
	Tags.Add(FName("Enemy")); 
	// AI 컨트롤러가 자동 빙의 하는거 제한
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	//Health = MaxHealth; // 시작할 때 체력을 최대 체력으로 설정

	if ( HealthBarWidget ) // 체력바 위젯에서 프로그레스바 설정
	{
		UEnemyHealthBarWidget* HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if ( HealthBar )
		{
			HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
		}
	}
	
	if ( bUseSpawnMontage == true && SpawnMontage ) // 스폰 몽타주 사용 시
	{
		PlayAnimMontage(SpawnMontage);
		// 체력바 안보이게
		if ( HealthBarWidget )
		{
			HealthBarWidget->SetVisibility(false);
		}
	}
	else
	{
		SpawnDefaultController();// 스폰 몽타주 사용 안하면 자동 빙의 설정
	}
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit();

	UpdateHealthBarWidget(DeltaTime); // 체력 바 위젯 업데이트 -> 항상 캐릭터 쪽으로 바라보도록
	
	if (bIsAttacking == true && EnemyType == EEnemyType::EET_Melee) // 근접 공격 타입이고 공격 중일 때
	{
		TArray<AActor*> OverlappingActors;
		// AttackRangePointSphere와 겹치는 모든 액터를 가져옵니다.
		AttackRangePointSphere->GetOverlappingActors(OverlappingActors);
		
		for (AActor* OverlappingActor : OverlappingActors)
		{
			// 액터가 유효하고 "Player" 태그를 가지고 있으며, 아직 공격한 목록에 없는지 확인합니다.
			if (OverlappingActor && OverlappingActor->ActorHasTag(FName("Player")) && !HittedActors.Contains(OverlappingActor))
			{
				// 공격 로그를 출력합니다.
				UE_LOG(LogTemp, Warning, TEXT("Attack Hit Detected on: %s"), *OverlappingActor->GetName());

				// 공격한 목록에 추가하여 중복 피해를 방지합니다.
				HittedActors.Add(OverlappingActor);

				bIsAttacking = false; // 공격 상태를 종료합니다.
				
				// 이 아래에 이제 대미지 넣는거 추가 가능
			}
		}
	}
}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseEnemy::PollInit()
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

// 포커스, 사운드, 애니메이션, 이펙트 등
void ABaseEnemy::Attack()
{
	//UE_LOG(LogTemp, Warning, TEXT("BaseEnemyAttackCall") );
	
	// 여기서 SetFoucs 하면 나중에 ClearFocus 도 해줘야함. 일단 커찮아서 안함.

	if ( AttackMontage ) // 공격 애니메이션 몽타주가 설정되어 있는지 확인
	{
		PlayAnimMontage(AttackMontage); // 공격 애니메이션 재생
	}
}

// 원거리 공격 몽타주에서 애님 노티파이로 호출
void ABaseEnemy::ShootProjectile()
{
	//UE_LOG(LogTemp, Warning, TEXT("BaseEnemyShootProjectileCall") );

	if (ProjectileClass && RangedAttackPoint && TargetCharacter)
	{
		const FVector SpawnLocation = RangedAttackPoint->GetComponentLocation();
		// RangedAttackPoint에서 TargetCharacter의 위치를 바라보는 회전값을 계산합니다.
		const FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetCharacter->GetActorLocation());

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		
		UWorld* World = GetWorld();
		if (World)
		{
			// 계산된 위치와 회전값으로 발사체를 스폰하고, 스폰된 액터의 포인터를 가져옵니다.
			ABaseEnemyProjectile* SpawnedProjectile = World->SpawnActor<ABaseEnemyProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

			// 엄
		}
	}
}

float ABaseEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Warning, TEXT("Enemy Take Damage : %f"), DamageToApply);

	if ( DamageToApply > 0.f )
	{
		Health -= DamageToApply;
		if ( Health <= 0.f )
		{
			Die();
		}
	}
	
	return DamageToApply;
}

void ABaseEnemy::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Die Call") );

	
	if ( DeathMontage ) // 죽음 애니메이션 몽타주가 설정되어 있는지 확인
	{
		PlayAnimMontage(DeathMontage); // 죽음 애니메이션 재생
	}

	// AI 컨트롤러를 가져옵니다.
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		// AI 컨트롤러에서 블랙보드 컴포넌트를 가져옵니다.
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(TEXT("IsDead"), true);
		}
	}
}

void ABaseEnemy::SpawnAndPossessAIController()
{
	// 애니메이셔 모드를 블루 프린트 모드로 설정
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	//UE_LOG(LogTemp, Warning, TEXT("SpawnAndPossessAIController Call") );
	SpawnDefaultController(); // 기본 AI 컨트롤러 스폰 및 빙의
	
	// 체력바 보이게
	if ( HealthBarWidget )
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void ABaseEnemy::ShowCharacterMesh()
{
	bUseSpawnMontage = false; // 스폰 몽타주 끝났음을 표시
}

void ABaseEnemy::UpdateHealthBarWidget(float DeltaTime)
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
	}
}

#if WITH_EDITOR
void ABaseEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// AttackRange 프로퍼티가 변경되었는지 확인합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, AttackRange))
	{
		if (AttackRangeSphere)
		{
			// AttackRangeSphere의 반지름을 AttackRange 값으로 설정합니다.
			AttackRangeSphere->SetSphereRadius(AttackRange);
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, DetectRange) )
	{
		if ( DetectRangeSphere )
		{
			DetectRangeSphere->SetSphereRadius(DetectRange);
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, ChaseRange) )
	{
		if ( ChaseRangeSphere )
		{
			ChaseRangeSphere->SetSphereRadius(ChaseRange);
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, MoveSpeed) )
	{
		if ( GetCharacterMovement() )
		{
			GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
		}
	}
}
#endif

