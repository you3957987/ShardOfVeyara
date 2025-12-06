#include "EnemyBoss/BaseBossEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "EnemyBoss/SkeletonMage/BossSkeletonMage.h"
#include "EnemyHUD/EnemyHealthBarWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

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

	Tags.Add(FName("Enemy")); // 적 캐릭터 태그 추가 -> 이걸 이용해서 프로젝트에서 플러그인 에너미 접근. 매우 중요!!!!
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
	
	TestDeadLogic();
}

void ABaseBossEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit(DeltaTime);
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
			bTargetInitalize = true; // 캐릭터가 유효하면 초기화 플래그를 true로 설정합니다.
		}
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

	UE_LOG(LogTemp, Warning, TEXT("BossSkeletonMage Take Damage : %f"), DamageToApply);

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
	Destroy(); // 이펙트가 없으면 바로 액터 삭제
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

void ABaseBossEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseBossEnemy, MoveSpeed) )
	{
		if ( GetCharacterMovement() )
		{
			GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
		}
	}
}

