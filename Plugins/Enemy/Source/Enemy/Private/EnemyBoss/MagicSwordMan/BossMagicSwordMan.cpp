#include "EnemyBoss/MagicSwordMan/BossMagicSwordMan.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "EnemyProjectile/BaseEnemyProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"

ABossMagicSwordMan::ABossMagicSwordMan()
{
	PowerAttackCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PowerAttackCollisionSphere"));
	PowerAttackCollisionSphere->SetupAttachment(GetRootComponent());
	PowerAttackCollisionSphere->SetSphereRadius( 540.f ); // 궁극기 공격 범위
	PowerAttackCollisionSphere->ShapeColor = FColor::Red;
	PowerAttackCollisionSphere->SetVisibility(false);
	PowerAttackCollisionSphere->SetHiddenInGame(false); 
	
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
	
	if ( PowerAttackCollisionSphere )
	{
		PowerAttackCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, 
			&ABossMagicSwordMan::OnBeginOverlapPowerAttackCollisionSphere);
		PowerAttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		if ( DamageWhileGuarding < AttackStruct.MaxDamageToReaction && GuardHitMontage )
		{
			PlayAnimMontage(GuardHitMontage);
		}
		
		UEnemyLogManager::EnemyLog(EEnemyLogType::BlackKnight, 
			FString::Printf(TEXT("[소드맨] 가드 성공 | 가드로 막은 대미지: [%.f] | 가드중 받은 대미지 / 반격 임계치[%.f / %.f]"), 
				DamageAmount, DamageWhileGuarding, AttackStruct.MaxDamageToReaction));
		
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
				HitCharacter->LaunchCharacter(FVector(0.f, 0.f, 900.f), false, true);
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
		
		if ( AttackType == EMagicSwordManAttackType::SimpleAttack )
		{
			UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 일반 공격 적중 | 대미지: [%.f]"), AttackDamage));
		}
		else if ( AttackType == EMagicSwordManAttackType::JumpUpAttack )
		{
			UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 띄우기 공격 적중 | 대미지: [%.f]"), AttackDamage));
		}
		else if ( AttackType == EMagicSwordManAttackType::AirAttack )
		{
			UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 공중 공격 적중 | 대미지: [%.f]"), AttackDamage));
		}
		else if ( AttackType == EMagicSwordManAttackType::JumpAttack )
		{
			UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 점프 공격 적중 | 대미지: [%.f]"), AttackDamage));
		} 
		else if ( AttackType == EMagicSwordManAttackType::GuardReaction )
		{
			UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 가드 반격 공격 적중 | 대미지: [%.f]"), AttackDamage));
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
			if (BlackboardComp) BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.CloseAttackDelay);
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
			if (BlackboardComp) BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.DashAttackDelay);
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
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("AirAttack", false);
			BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.CloseJumpUpAttackDelay);
		}
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
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool("AirAttack", false);
			BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.DashJumpUpAttackDelay);
		}
		return DashJumpUpAttackMontage;
	}
	return nullptr;
}

void ABossMagicSwordMan::JumpUpAttackCheck()
{
	UE_LOG( LogTemp, Warning, TEXT("JumpUpAttackCheck called. bSuccessJumpUpAttack: %s"), bSuccessJumpUpAttack ? TEXT("true") : TEXT("false") );
	
	UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, 
		FString::Printf(TEXT("[소드맨] 띄우기 성공 여부: [%s]"), bSuccessJumpUpAttack ? TEXT("성공") : TEXT("실패")));
	
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	if ( bSuccessJumpUpAttack == true )
	{
		// 띄우기 성공 시 공중 공격 패턴으로 전환
		BlackboardComp->SetValueAsBool("AirAttack", true);
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
			AttackType = EMagicSwordManAttackType::JumpAttack;
			if (BlackboardComp) BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.JumpAttackDelay);
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

UAnimMontage* ABossMagicSwordMan::StartGuardReactionAttack()
{
	if (GuardReactionMontages.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, GuardReactionMontages.Num() - 1);
		
		if (GuardReactionMontages[RandomIndex])
		{
			PlayAnimMontage(GuardReactionMontages[RandomIndex]);
			AttackType = EMagicSwordManAttackType::GuardReaction;
			return GuardReactionMontages[RandomIndex];
		}
	}
	return nullptr;
}

UAnimMontage* ABossMagicSwordMan::StartPowerAttack()
{
	if (PowerAttackMontage)
	{
		PlayAnimMontage(PowerAttackMontage);
		bIsPowerAttackHit = false; // 공격 시작 시점에는 궁극기 공격이 적중했는지 여부를 false로 초기화
		if (BlackboardComp) BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.PowerAttackDelay);
		return PowerAttackMontage;
	}
	return nullptr;
}

void ABossMagicSwordMan::OnBeginOverlapPowerAttackCollisionSphere(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		HandlePowerAttackDamage(OtherActor);
		
		bIsPowerAttackHit = true; // 궁극기 공격이 적중했음을 표시하는 플래그를 true로 설정
		
		UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 궁극기 공격 적중 ")));
		
		// 타겟 정지 및 입력 차단 로직 시작 
		if (ACharacter* HitCharacter = Cast<ACharacter>(OtherActor))
		{
			//  플레이어 컨트롤러를 가져와서 입력을 비활성화
			if (APlayerController* PlayerController = Cast<APlayerController>(HitCharacter->GetController()))
			{
				HitCharacter->DisableInput(PlayerController);
			}

			//무브먼트 컴포넌트를 가져와서 움직임을 정지
			if (UCharacterMovementComponent* MovementComp = HitCharacter->GetCharacterMovement())
			{
				MovementComp->StopMovementImmediately(); // 현재 속도와 가속도를 0으로 만듦
				MovementComp->SetMovementMode(EMovementMode::MOVE_None); // 이동 모드를 끄면 중력 영향을 받지 않고 그 자리에 멈춤
			}
			// 애니메이션도 멈추기 
			if (USkeletalMeshComponent* StopMesh = HitCharacter->GetMesh())
			{
				StopMesh->bPauseAnims = true;
			}
		}
		
		// 궁극기 공격이 한 번 적중하면 콜리전 비활성화 
		PowerAttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABossMagicSwordMan::HandlePowerAttackDamage(AActor* OtherActor)
{
	if (!OtherActor) return;

	// 1. 반응성을 위해 즉시 1회 대미지 적용 (선택 사항)
	UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());

	UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 궁극기 대미지 적용 | 대미지: [%.f]"), AttackDamage));
	
	// 2. 파라미터(OtherActor)를 전달하기 위해 델리게이트 생성
	FTimerDelegate TimerCallback;
	TimerCallback.BindUObject(this, &ABossMagicSwordMan::OnPowerAttackTimerTick, OtherActor);

	// 3. 0.3초 간격으로 반복(true) 실행되는 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(PowerAttackTimerHandle, TimerCallback, 0.05f, true);
}

void ABossMagicSwordMan::StartPowerAttackCollision()
{
	if ( PowerAttackCollisionSphere )
	{
		PowerAttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ABossMagicSwordMan::EndPowerAttackCollision()
{
	if ( PowerAttackCollisionSphere )
	{
		PowerAttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// 공격이 끝나면 타이머를 해제하여 대미지를 멈춤
	GetWorld()->GetTimerManager().ClearTimer(PowerAttackTimerHandle);
}

void ABossMagicSwordMan::OnPowerAttackTimerTick(AActor* TargetActor)
{
	// 타겟이 유효한지(파괴되지 않았는지) 확인
	if (IsValid(TargetActor))
	{
		// 주기적 대미지 적용
		UGameplayStatics::ApplyDamage(TargetActor, AttackStruct.PowerAttackTickDamage, 
			GetController(), this, UDamageType::StaticClass());
		// 대미지 로그
		UE_LOG(LogTemp, Warning, TEXT("Boss Power Attack Tick Damage : %f"), AttackStruct.PowerAttackTickDamage);
	}
	else
	{
		// 타겟이 유효하지 않으면 타이머 정지
		GetWorld()->GetTimerManager().ClearTimer(PowerAttackTimerHandle);
	}
}

void ABossMagicSwordMan::FinishPowerAttack()
{
	if ( bIsPowerAttackHit )
	{
		UGameplayStatics::ApplyDamage(TargetCharacter, AttackStruct.PowerAttackFinishDamage, 
			GetController(), this, UDamageType::StaticClass());
		
		UEnemyLogManager::EnemyLog(EEnemyLogType::MagicSwordMan, FString::Printf(TEXT("[소드맨] 궁극기 마무리 대미지 적용 | 대미지: [%.f]"), AttackStruct.PowerAttackFinishDamage));
		
		// 피니쉬 대미지 로그
		UE_LOG(LogTemp, Warning, TEXT("Boss Power Attack Finish Damage : %f"), AttackStruct.PowerAttackFinishDamage);
		
		// 타겟 움직임 및 입력 복구 
		if (ACharacter* HitCharacter = Cast<ACharacter>(TargetCharacter))
		{
			// 이동 모드 복구 
			if (UCharacterMovementComponent* MovementComp = HitCharacter->GetCharacterMovement())
			{
				MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
			}

			// 플레이어 입력 다시 활성화
			if (APlayerController* PlayerController = Cast<APlayerController>(HitCharacter->GetController()))
			{
				HitCharacter->EnableInput(PlayerController);
			}
			
			// 애니메이션 재생 다시 활성화
			if (USkeletalMeshComponent* StopMesh = HitCharacter->GetMesh())
			{
				StopMesh->bPauseAnims = false;
			}
		}
	}
}

UAnimMontage* ABossMagicSwordMan::StartBladeWaveAttack()
{
	if (BladeWaveAttackMontage)
	{
		PlayAnimMontage(BladeWaveAttackMontage);
		AttackType = EMagicSwordManAttackType::SimpleAttack;
		if (BlackboardComp) BlackboardComp->SetValueAsFloat("AttackDelay", AttackStruct.BladeWaveAttackDelay);
		return BladeWaveAttackMontage;
	}
	return nullptr;
}

void ABossMagicSwordMan::StartBladeWave()
{
	if (BladeWaveProjectileClass)
	{
		// 정면으로 발사체 스폰 . 약간 앞에서만
		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 150.f;
		FRotator SpawnRotation = GetActorRotation();
		GetWorld()->SpawnActor<ABaseEnemyProjectile>(BladeWaveProjectileClass, SpawnLocation, SpawnRotation);
	}
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
			if ( PowerAttackCollisionSphere ) PowerAttackCollisionSphere->SetVisibility(true);
		}
		else
		{
			if ( PowerAttackCollisionSphere ) PowerAttackCollisionSphere->SetVisibility(false);
		}
	}
}
#endif