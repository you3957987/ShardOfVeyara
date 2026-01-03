// Fill out your copyright notice in the Description page of Project Settings.


#include "Crop.h"
#include "Components/sphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AGSDCharacter.h"
#include "AGSDPlayerController.h"

// Sets default values
ACrop::ACrop()
{
 	// Set this actor to call Tick() ever	y frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//루트 컴포넌트 설정
	CropMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CropMesh"));
	RootComponent = CropMesh;

	CropMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECR_Overlap
		);
	CropMesh->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Camera,
		ECR_Ignore
		);

	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetSphereRadius(50);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ACrop::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ACrop::OnEndOverlap);
}

//오버랩 시작 함수 구현부
void ACrop::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->AddInteractableActor(this);
	}
}

//오버랩 종료 함수 구현부
void ACrop::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

// Called when the game starts or when spawned
void ACrop::BeginPlay()
{
	Super::BeginPlay();
	// 현재 메쉬의 월드 좌표 (X, Y는 유지하고 Z만 바꿀 것임)
	FVector MeshLoc = CropMesh->GetComponentLocation();

	// 3. 레이저 쏘기 설정 (내 머리 위 500 ~ 내 발 밑 500)
	FVector TraceStart = FVector(MeshLoc.X, MeshLoc.Y, MeshLoc.Z + 500.0f);
	FVector TraceEnd   = FVector(MeshLoc.X, MeshLoc.Y, MeshLoc.Z - 500.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 내 자신(잡초 뭉치)은 무시

	// 4. 레이저 발사!
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_WorldStatic, // 지형(WorldStatic)만 체크
		Params
	);

	if (bHit)
	{
		// 5. 땅에 닿았다면 위치 이동 (World Location 설정)
		CropMesh->SetWorldLocation(HitResult.Location);

		// (선택 사항) 경사면에 맞춰 기울이기
		// 나무가 아니라 납작한 풀이라면 기울이는 게 자연스럽습니다.
		// 필요 없다면 이 줄은 지우세요.
		FRotator CurrentRot = CropMesh->GetComponentRotation();
            
		// 바닥의 기울기(Normal)를 회전값으로 변환하되, Z축 회전(Yaw)은 원래 랜덤하게 돌려놓은 값을 유지
		FRotator TargetRot = HitResult.ImpactNormal.Rotation();
		TargetRot.Yaw = CurrentRot.Yaw; // 원래 회전각 유지
		TargetRot.Pitch -= 90.0f;       // Normal은 수직이 기준이라 눕혀줘야 할 수도 있음 (메쉬 축에 따라 다름)
            
		// 간단하게는 위 코드 대신 아래처럼 Normal에 UpVector를 맞추는 방식을 많이 씁니다.
		FRotator AlignRot = FRotationMatrix::MakeFromZ(HitResult.ImpactNormal).Rotator();
		CropMesh->SetWorldRotation(AlignRot);
	}
}

//작물 수확 구현부
void ACrop::HarvestCrop()
{
	if (HarvestClass == nullptr) return;
	
	const float SpawnRadius = 50.f;
	const FVector TargetLocation = GetActorLocation();

	float RandomAngle = FMath::RandRange(0.0f, 360.f);
	float RandomDist = FMath::RandRange(0.f, SpawnRadius);

	FVector SpawnOffset(
		RandomDist * FMath::Cos(RandomAngle),
		RandomDist * FMath::Sin(RandomAngle),
		0.0f
		);

	FVector FinalSpawnLocation = TargetLocation + SpawnOffset;

	FTransform SpawnTransform = GetTransform();
	SpawnTransform.SetLocation(FinalSpawnLocation + FVector(0.f, 0.f, 40.f));
	
	APickUpItem* Harvest = GetWorld()->SpawnActorDeferred<APickUpItem>(
	CropData->HarvestRewards[0].Harvest,
	SpawnTransform,
	this,
	nullptr,
	ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);
	if (Harvest)
	{
		UGameplayStatics::FinishSpawningActor(Harvest, SpawnTransform);
		//초기 선형 속도 설정: Z축(위) 방향으로만 작은 속도를 줌
		if (Harvest->GetMeshComponent()) // Harvest 액터에 MeshComponent를 가져오는 함수가 있다고 가정
		{
			// 50.0f 정도의 작은 힘으로 위로 튀어 오르게 합니다.
			Harvest->GetMeshComponent()->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, 100.0f)); 
		}
	}
}

void ACrop::SetCropData(UUCropData* CData)
{
	if (CData != nullptr)
	{
		this->CropData = CData;
	}
}

//상호작용 시 구현부
void ACrop::Interact_Implementation(AAGSDCharacter* player)
{
	UE_LOG(LogTemp, Warning, TEXT("ACrop::OnBeginOverlap"));

	for (int i = 0; i < CropData->HarvestRewards[0].Quantity; i++)	HarvestCrop();
	if (OnHarvested.IsBound())
	{
		OnHarvested.Broadcast();
	}
	Destroy();
}

void ACrop::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool ACrop::CanInteract_Implementation(AAGSDCharacter* player)
{
	return true;
}

void ACrop::MeshUpdate(int32 currentGrowStageIndex)
{
	if (CropData != nullptr && CropData->GrowthStages[currentGrowStageIndex].Mesh)
	{
		CropMesh->SetStaticMesh(CropData->GrowthStages[currentGrowStageIndex].Mesh);
	}
}

void ACrop::SetCollisionEnable()
{
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}