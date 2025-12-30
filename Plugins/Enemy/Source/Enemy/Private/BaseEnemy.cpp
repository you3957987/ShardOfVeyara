#include "BaseEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Progressbar.h"
#include "Kismet/GameplayStatics.h" // UGameplayStatics 사용을 위한 헤더 파일
#include "Components/SphereComponent.h" // USphereComponent 사용을 위한 헤더 파일
#include "Components/WidgetComponent.h"
#include "EnemyHUD/EnemyHealthBarWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트
	AttackRangeSphere->ShapeColor = FColor::Red;
	AttackRangeSphere->SetSphereRadius(AttackRange); // 초기 공격 범위 설정
	AttackRangeSphere->SetVisibility(false); // 디버그 모드 기본은 비활성화
	AttackRangeSphere->SetHiddenInGame(false);
	
	DetectRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AcceptRangeSphere"));
	DetectRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트
	DetectRangeSphere->ShapeColor = FColor::Green;
	DetectRangeSphere->SetSphereRadius(DetectRange);
	DetectRangeSphere->SetVisibility(false);
	DetectRangeSphere->SetHiddenInGame(false); 
	
	ChaseRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ChaseRangeSphere"));
	ChaseRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트
	ChaseRangeSphere->ShapeColor = FColor::Blue;
	ChaseRangeSphere->SetSphereRadius(ChaseRange);
	ChaseRangeSphere->SetVisibility(false);
	ChaseRangeSphere->SetHiddenInGame(false); 
	
	// 체력 바 위젯 컴포넌트 생성 및 설정
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World); // 월드 공간으로 변경

	// 캐릭터 메시의 콜리전을 비활성화합니다.
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	// 캡슐 컴포넌트가 카메라에 반응하지 않도록 설정합니다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	// 적 캐릭터 태그 추가 -> 이걸 이용해서 프로젝트에서 플러그인 에너미 접근. 매우 중요!!!!
	Tags.Add(FName("Enemy")); 
	// AI 컨트롤러가 자동 빙의 하는거 제한. BeginPlay에서 스폰 몽타주 사용 여부에 따라 설정
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
	TestDeadLogic(); // 죽음 로직 테스트 함수
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit();

	UpdateHealthBarWidget(DeltaTime); // 체력 바 위젯 업데이트 -> 항상 캐릭터 쪽으로 바라보도록
	
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
	// 여기서 SetFoucs 하면 나중에 ClearFocus 도 해줘야함. 일단 커찮아서 안함.

	if ( AttackMontage ) // 공격 애니메이션 몽타주가 설정되어 있는지 확인
	{
		PlayAnimMontage(AttackMontage); // 공격 애니메이션 재생
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
	if ( DeathMontage ) PlayAnimMontage(DeathMontage);
	
	// 충돌 비활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	// AI 로직 중지
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(TEXT("IsDead"), true);
		}
	}
	
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void ABaseEnemy::AfterDieMontageEnd()
{
	if ( GetMesh() )
	{
		GetMesh()->bPauseAnims = true;
	}
	
	// 0.3초 후에 SpawnEffectAndDestroy 함수를 호출합니다.
	GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, this,
		&ABaseEnemy::SpawnDeadEffectAndDestroy, 1.0f, false);
}

void ABaseEnemy::SpawnDeadEffectAndDestroy()
{
	if ( DeathEffectCascade )
	{
		const FVector SpawnLocation = GetMesh()->GetComponentLocation() + (GetActorForwardVector() * DeathEffectForwardOffset);
		const FRotator SpawnRotation = GetActorRotation();
		const FVector SpawnScale = FVector(DeathEffectScale);

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathEffectCascade,
		 SpawnLocation, SpawnRotation, SpawnScale);
	}
	Destroy(); // 이펙트가 없으면 바로 액터 삭제
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

		UEnemyHealthBarWidget* HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
		if ( HealthBar )
		{
			HealthBar->HealthProgressBar->SetPercent(Health / MaxHealth);
		}
	}
}

void ABaseEnemy::TestDeadLogic()
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

#if WITH_EDITOR
void ABaseEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( AttackRangeSphere ) AttackRangeSphere->SetVisibility(true);
			if ( DetectRangeSphere ) DetectRangeSphere->SetVisibility(true);
			if ( ChaseRangeSphere ) ChaseRangeSphere->SetVisibility(true);
		}
		else
		{
			if ( AttackRangeSphere ) AttackRangeSphere->SetVisibility(false);
			if ( DetectRangeSphere ) DetectRangeSphere->SetVisibility(false);
			if ( ChaseRangeSphere ) ChaseRangeSphere->SetVisibility(false);
		}
	}
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

