#include "EnemyBoss/MagicSwordMan/BossMagicSwordMan.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h" 
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
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag(FName("Player")))
	{
		// 어택 대미지 로그 
		UE_LOG(LogTemp, Warning, TEXT("Guard Enemy Attack Damage : %f"), AttackDamage);

		// 대미지 적용 ( 어택 대미지는 공격 전 가드 공격인지 아님 일반 공격인지에 따라 각각 함수에서 설정 )
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(),
			this, UDamageType::StaticClass());


		// 다시 콜리전 끄기
		if ( WeaponCollision ) WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
			UE_LOG(LogTemp, Log, TEXT("Playing Random Melee Attack Index: %d"), RandomIndex);
			return CloseAttackMontages[RandomIndex];
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("array is EMPTY"));
	return nullptr;
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