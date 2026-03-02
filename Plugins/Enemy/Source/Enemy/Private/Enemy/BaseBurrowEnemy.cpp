#include "Enemy/BaseBurrowEnemy.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseBurrowEnemy::ABaseBurrowEnemy()
{
	// 최대 걷기 속도 제로로 해서 움직이지 않도록 설정
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;
	
}

void ABaseBurrowEnemy::BeginPlay()
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
			AttackCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseBurrowEnemy::OnBeginOverlapAttackCollisionSphere);
			// 콜리전 끄기
			AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break; // 찾았으니 루프 종료
		}
	}
	
}

void ABaseBurrowEnemy::Tick(float DeltaTime)
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

		// 대미지 적용 ( 어택 대미지는 공격 전 가드 공격인지 아님 일반 공격인지에 따라 각각 함수에서 설정 )
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(),
			this, UDamageType::StaticClass());


		// 다시 콜리전 끄기
		if ( AttackCollisionSphere ) AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABaseBurrowEnemy::AttackStart_AttackCollisionSphere()
{
	if ( AttackCollisionSphere ) AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ABaseBurrowEnemy::AttackEnd_AttackCollisionSphere()
{
	if ( AttackCollisionSphere ) AttackCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

void ABaseBurrowEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
}
