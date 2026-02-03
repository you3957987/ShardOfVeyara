#include "EnemySpawner/EnemySpaner.h"

#include "Components/SphereComponent.h"

AEnemySpaner::AEnemySpaner()
{
	PrimaryActorTick.bCanEverTick = true;

	RootCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RootCollisionSphere"));
	RootComponent = RootCollisionSphere;

	SpawnerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnerMesh"));
	SpawnerMesh->SetupAttachment(RootComponent);
	SpawnerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent);
	
}

void AEnemySpaner::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemySpaner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

