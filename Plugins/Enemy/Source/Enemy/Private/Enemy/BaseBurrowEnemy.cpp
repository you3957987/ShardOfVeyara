#include "Enemy/BaseBurrowEnemy.h"

#include "EnemyLogManager.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseBurrowEnemy::ABaseBurrowEnemy()
{
	// 최대 걷기 속도 제로로 해서 움직이지 않도록 설정
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;
	
	// 이동 불가 
	GetCharacterMovement()->MaxWalkSpeed = 0.0f; 
	
	HealthBarWidget->SetVisibility(false);
	
	DetectRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChaseRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	MeleeAttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttackPoint"));
	MeleeAttackPoint->SetupAttachment(RootComponent); // 루트 컴포넌트
	
	AttackRangePointSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangePointSphere"));
	AttackRangePointSphere->SetupAttachment(MeleeAttackPoint); // AttackPoint에 부착
	AttackRangePointSphere->ShapeColor = FColor::Purple;
	AttackRangePointSphere->SetVisibility(false);
	AttackRangePointSphere->SetHiddenInGame(false);
	
	BurrowAttackCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("BurrowAttackCollisionSphere"));
	BurrowAttackCollisionSphere->SetupAttachment(RootComponent);
	BurrowAttackCollisionSphere->ShapeColor = FColor::Black;
	BurrowAttackCollisionSphere->SetVisibility(false);
	BurrowAttackCollisionSphere->SetHiddenInGame(false);
	
	EnemyType = EEnemyType::EET_Burrow;
}

void ABaseBurrowEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<USphereComponent*> SphereComps;
	GetComponents<USphereComponent>(SphereComps);
	
	if ( AttackRangePointSphere )
	{
		AttackRangePointSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseBurrowEnemy::OnBeginOverlapAttackCollisionSphere);
		AttackRangePointSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
		
	if ( DetectRangeSphere ) DetectRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseBurrowEnemy::OnBeginOverlapDetectRangeSphere);
	
	if ( ChaseRangeSphere ) ChaseRangeSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseBurrowEnemy::OnEndOverlapChaseRangeSphere);
}

void ABaseBurrowEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float ABaseBurrowEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABaseBurrowEnemy::OnBeginOverlapAttackCollisionSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack Sphere Overlapped with Player!"));

		// 로그 기록 로직
		if (GetMesh()) 
		{
			// 스켈레탈 메쉬 에셋 이름 가져오기
			FString MeshName = GetMesh()->GetSkeletalMeshAsset() ? GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");
					
			UEnemyLogManager::EnemyLog(EEnemyLogType::Burrow, 
				FString::Printf(TEXT("적 [%s]가 [%.f] 대미지 - 기본 공격"), 
					*MeshName, 
					AttackDamage));
		}
		
		// 대미지 적용 ( 어택 대미지는 공격 전 가드 공격인지 아님 일반 공격인지에 따라 각각 함수에서 설정 )
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(),
			this, UDamageType::StaticClass());
		
		// 다시 콜리전 끄기
		if ( AttackRangePointSphere ) AttackRangePointSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABaseBurrowEnemy::OnBeginOverlapDetectRangeSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		if ( bIsBurrowing == true )
		{
			PlayUnburrowMontage();
		}
	}
}

void ABaseBurrowEnemy::OnEndOverlapChaseRangeSphere(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		if ( bIsBurrowing == false )
		{
			PlayBurrowMontage();
		}
	}
}

void ABaseBurrowEnemy::AttackStart_AttackCollisionSphere()
{
	if ( AttackRangePointSphere ) AttackRangePointSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABaseBurrowEnemy::AttackEnd_AttackCollisionSphere()
{
	if ( AttackRangePointSphere ) AttackRangePointSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseBurrowEnemy::PlayBurrowMontage()
{
	if ( BurrowMontage )
	{
		PlayAnimMontage(BurrowMontage);
	}
}

void ABaseBurrowEnemy::PlayUnburrowMontage()
{
	if ( UnburrowMontage )
	{
		PlayAnimMontage(UnburrowMontage);
	}
}

void ABaseBurrowEnemy::FinishUnburrow()
{
	bIsBurrowing = false;
	
	if ( HealthBarWidget ) HealthBarWidget->SetVisibility(true);

	if ( BurrowAttackCollisionSphere )
	{
		// 플레이어 액터 감지 
		TArray<AActor*> OverlappingActors;
		BurrowAttackCollisionSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass()); // 캐릭터 클래스만 필터링

		for (AActor* Actor : OverlappingActors)
		{
			// 나 자신이 아니고, Player 태그가 있는지 확인
			if (Actor && Actor != this && Actor->ActorHasTag(FName("Player")))
			{
				ACharacter* PlayerCharacter = Cast<ACharacter>(Actor);
				if (PlayerCharacter)
				{
					const float PushForce = 700.0f;       // 밀어내는 힘 
					const float PushUpwardForce = 200.0f;  // 띄우는 힘 

					// 밀어낼 방향 계산 (적 -> 플레이어)
					FVector PushDirection = PlayerCharacter->GetActorLocation() - GetActorLocation();
					PushDirection.Z = 0; // 수직 방향 영향 제거
					PushDirection.Normalize();

					// 밀어낼 속도 계산
					const FVector LaunchVelocity = PushDirection * PushForce + FVector(0.f, 0.f, PushUpwardForce);

					// 플레이어 캐릭터를 밀쳐냄 (XY, Z 모두 강제 적용 override)
					PlayerCharacter->LaunchCharacter(LaunchVelocity, true, true);
					
					// 로그 기록 로직
					if (GetMesh()) 
					{
						// 스켈레탈 메쉬 에셋 이름 가져오기
						FString MeshName = GetMesh()->GetSkeletalMeshAsset() ? GetMesh()->GetSkeletalMeshAsset()->GetName() : TEXT("NoMeshAsset");
					
						UEnemyLogManager::EnemyLog(EEnemyLogType::Burrow, 
							FString::Printf(TEXT("적 [%s]가 [%.f] 대미지 - 언버로우"), 
								*MeshName, 
								AttackDamage));
					}
					
					// 대미지 적용 (기존 AttackDamage 사용)
					UGameplayStatics::ApplyDamage(PlayerCharacter, 10, GetController(),
						this, UDamageType::StaticClass());
					
					// 로그 출력
					UE_LOG(LogTemp, Warning, TEXT("[BaseBurrowEnemy] Unburrow Attack Hit Player!"));
				}
			}
		}
	}
}

void ABaseBurrowEnemy::FinishBurrow()
{
	bIsBurrowing = true;
	
	if ( HealthBarWidget ) HealthBarWidget->SetVisibility(false);
}

#if WITH_EDITOR
void ABaseBurrowEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseBurrowEnemy, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( AttackRangePointSphere ) AttackRangePointSphere->SetVisibility(true);
			if (BurrowAttackCollisionSphere) BurrowAttackCollisionSphere->SetVisibility(true);
		}
		else
		{
			if ( AttackRangePointSphere ) AttackRangePointSphere->SetVisibility(false);
			if ( BurrowAttackCollisionSphere ) BurrowAttackCollisionSphere->SetVisibility(false);
		}
	}
}
#endif