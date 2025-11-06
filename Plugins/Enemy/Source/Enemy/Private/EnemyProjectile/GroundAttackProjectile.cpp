#include "EnemyProjectile/GroundAttackProjectile.h"


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
}

void AGroundAttackProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
			UE_LOG(LogTemp, Warning, TEXT("Projectile hit Player: %s"), *OtherActor->GetName());

			// 첫 오버랩 이후 MeshComp의 콜리전을 비활성화합니다.
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

