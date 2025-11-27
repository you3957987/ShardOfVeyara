#include "EnemyProjectile/GroundAttackProjectile.h"

#include "Kismet/GameplayStatics.h"


AGroundAttackProjectile::AGroundAttackProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent); // 루트 컴포넌트에 부착
	
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AGroundAttackProjectile::BeginPlay()
{
	Super::BeginPlay();

	if ( MeshComp )
	{
		MeshComp->OnComponentBeginOverlap.AddDynamic(this, &AGroundAttackProjectile::OnBeginOverlap);
	}

	// 생성 후 상승 로직 시작
	if (RiseDuration > 0.f )
	{
		InitialLocation = GetActorLocation();
		bIsRising = true;
	}
}

void AGroundAttackProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRising == true)
	{
		UpperMesh(DeltaTime);
	}
	else if (bIsStaying == true)
	{
		StayElapsedTime += DeltaTime;
		if (StayElapsedTime >= DurationTime)
		{
			bIsStaying = false;
			// 하강 시간이 있으면 하강 시작, 없으면 바로 파괴
			if (RiseDuration > 0.f)
			{
				bIsLowering = true;
			}
			else
			{
				Destroy();
			}
		}
	}
	else if (bIsLowering == true)
	{
		//UE_LOG(LogTemp, Warning, TEXT("aaa"));
		LowerMesh(DeltaTime);
	}
}

void AGroundAttackProjectile::UpperMesh(float DeltaTime)
{
	if (bIsRising == true)
	{
		RiseElapsedTime += DeltaTime;
		if (RiseElapsedTime < RiseDuration)
		{
			// 시간에 따라 목표 높이까지 상승
			const float NewZ = InitialLocation.Z + RiseHeight * (RiseElapsedTime / RiseDuration);
			FVector NewLocation = GetActorLocation();
			NewLocation.Z = NewZ;
			SetActorLocation(NewLocation);
			//UE_LOG(LogTemp, Warning, TEXT("aaaa"));
		}
		else
		{
			// 상승 완료
			bIsRising = false;
			// 최종 위치 설정
			FVector FinalLocation = GetActorLocation();
			FinalLocation.Z = InitialLocation.Z + RiseHeight;
			SetActorLocation(FinalLocation);

			// 최종 위치를 PeakLocation에 저장합니다.
			PeakLocation = InitialLocation + FVector(0.f, 0.f, RiseHeight);
			bIsStaying = true;
		}
	}
}

void AGroundAttackProjectile::LowerMesh(float DeltaTime)
{
	LowerElapsedTime += DeltaTime;
	if (LowerElapsedTime < LowerDuration)
	{
		// Lerp의 Alpha 값을 계산할 때 RiseDuration 대신 LowerDuration을 사용합니다.
		const float NewZ = FMath::Lerp(PeakLocation.Z, InitialLocation.Z, LowerElapsedTime / LowerDuration);
		FVector NewLocation = GetActorLocation();
		NewLocation.Z = NewZ;
		SetActorLocation(NewLocation);
	}
	else
	{
		bIsLowering = false;
		SetActorLocation(InitialLocation);
		//UE_LOG(LogTemp, Warning, TEXT("Ground Attack Projectile Lowering Complete"));
		Destroy(); // 하강 완료 후 액터 파괴
	}
}

void AGroundAttackProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이나 소유자와 충돌한 경우는 무시합니다.
	if (OtherActor && (OtherActor != this) && (OtherActor != GetOwner()))
	{
		// 충돌한 액터가 "Player" 태그를 가지고 있는지 확인합니다.
		if (OtherActor->ActorHasTag(FName("Player")))
		{
			UE_LOG(LogTemp, Warning, TEXT("GroundAttack hit Player: %s"), *OtherActor->GetName());

			// 플레이어에게 대미지를 적용합니다.
			UGameplayStatics::ApplyDamage(
			 OtherActor,
			 Damage, 
			 GetOwner() ? GetOwner()->GetInstigatorController() : nullptr, // 소유자의 컨트롤러를 데미지 인스티게이터로 사용
			this,
			 UDamageType::StaticClass()
			);
			
			// 첫 오버랩 이후 MeshComp의 콜리전을 비활성화합니다.
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

