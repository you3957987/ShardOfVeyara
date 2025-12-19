#include "BaseFlyingPet.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABaseFlyingPet::ABaseFlyingPet()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	// 카메라 채널 무시 설정
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	// 메쉬는 충돌을 처리하지 않음
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FloatingMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));

	// 적 감지 범위 구체 컴포넌트 생성 및 설정
	EnemyDetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EnemyDetectSphere"));
	EnemyDetectSphere->SetupAttachment(RootComponent);
	EnemyDetectSphere->ShapeColor = FColor::Green;
	EnemyDetectSphere->SetSphereRadius(EnemyDetectRange);
	EnemyDetectSphere->SetVisibility(false);
	EnemyDetectSphere->SetHiddenInGame(false); 

	// 아이템 감지 범위 구체 컴포넌트 생성 및 설정
	ItemDetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ItemDetectSphere"));
	ItemDetectSphere->SetupAttachment(RootComponent);
	ItemDetectSphere->ShapeColor = FColor::Blue;
	ItemDetectSphere->SetSphereRadius(ItemDetectRange);
	ItemDetectSphere->SetVisibility(false);
	ItemDetectSphere->SetHiddenInGame(false);
	
	// 펫 태그 추가
	Tags.Add("Pet");
}

void ABaseFlyingPet::BeginPlay()
{
	Super::BeginPlay();

	if ( ItemDetectSphere )
	{
		// 비긴 오버랩 이벤트 바인딩
		ItemDetectSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseFlyingPet::OnItemDetectBeginOverlap);
	}
	if ( EnemyDetectRange )
	{
		
	}
}

void ABaseFlyingPet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit(DeltaTime);

	if ( TargetActor && bIsFolloingTarget == true )
	{
		FollowingTarget(DeltaTime);
	}
}

void ABaseFlyingPet::PollInit(float DeltaTime)
{
	if ( bTargetInitalize == false )
	{
		// 더 안전하고 깔끔한 방식
		TargetActor = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        
		if ( TargetActor ) // 타겟 액터가 유효한지 확인합니다.
		{
			bTargetInitalize = true; // 타겟이 유효하면 초기화 플래그를 true로 설정합니다.
			bIsFolloingTarget = true; // 타겟이 설정되면 따라다니기 모드 활성화
		}
	}
}

void ABaseFlyingPet::SetFreeRoaming(bool bNewState)
{
	bIsFolloingTarget = bNewState;
}

void ABaseFlyingPet::FollowingTarget(float DeltaTime)
{
	if (!TargetActor) return;

	// --- [핵심] 상태(PetState)에 따른 설정값 선택 ---
	FPetPositionSettings CurrentSettings;
	switch (PetState)
	{
		case EPetState::EPS_Conversation:
			CurrentSettings = ConversationSettings;
			break;
		case EPetState::EPS_Follow:
			CurrentSettings = FollowSettings;
				break;
		default:
			CurrentSettings = FollowSettings;
			break;
	}

	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();

	// --- 1. 목표 위치 계산 ---
	FVector TargetForward = TargetActor->GetActorForwardVector();
	FVector TargetRight = TargetActor->GetActorRightVector();

	// 공식 == 타겟 위치 - (앞방향 * 거리) + (오른쪽방향 * 좌우오프셋) + (위방향 * 높이오프셋)
	FVector DesiredLocation = TargetLocation
		- (TargetForward * CurrentSettings.Distance)    // 거리
		+ (TargetRight * CurrentSettings.SideOffset)    // 좌우
		+ FVector(0.0f, 0.0f, CurrentSettings.UpOffset); // 높이

	// 위치 보간 (부드럽게 이동)
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaTime, MoveInterpSpeed);
	SetActorLocation(NewLocation);

	 // --- 2. 회전 계산 (상태에 따른 분기) ---
    FRotator TargetRotation;

    if (PetState == EPetState::EPS_Conversation)
    {
        // [대화 상태] 펫이 캐릭터를 바라보도록 회전 계산 (LookAt)
        TargetRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
        
        // (선택사항) 대화 시에는 펫이 캐릭터를 정면으로 보기 위해 180도 돌려야 할 수도 있습니다.
        // 만약 펫이 캐릭터를 등지고 대화한다면 아래 주석을 해제하세요.
        // TargetRotation.Yaw += 180.0f; 
    }
    else if ( PetState == EPetState::EPS_Follow )
    {
        TargetRotation = TargetActor->GetActorRotation();
    }

    // [공통] 상하 기울기(Pitch)와 좌우 기울기(Roll)는 0으로 고정 (수평 유지)
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;

    // 회전 보간
    FRotator CurrentRotation = GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, MoveInterpSpeed);
    SetActorRotation(NewRotation);
}

void ABaseFlyingPet::OnItemDetectBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 겹친 액터가 Item 태그를 가지고 있는지 확인
	if ( OtherActor && OtherActor != this && OtherActor->ActorHasTag("Item") )
	{
		// 아이템 획득 로직 구현 (예: 아이템 파괴)
		UE_LOG(LogTemp, Warning, TEXT("Item Detected: %s"), *OtherActor->GetName());
	}
}

void ABaseFlyingPet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// 디버그 모드에 따라 어택, 디텍트, 체이스 범위 구체의 가시성을 설정합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseFlyingPet, bDebugMode))
	{
		if ( bDebugMode == true )
		{
			if ( EnemyDetectSphere ) EnemyDetectSphere->SetVisibility(true);
			if ( ItemDetectSphere ) ItemDetectSphere->SetVisibility(true);
		}
		else
		{
			if ( EnemyDetectSphere ) EnemyDetectSphere->SetVisibility(false);
			if ( ItemDetectSphere ) ItemDetectSphere->SetVisibility(false);
		}
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseFlyingPet, EnemyDetectRange))
	{
		if (EnemyDetectSphere)
		{
			EnemyDetectSphere->SetSphereRadius(EnemyDetectRange);
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseFlyingPet, ItemDetectRange) )
	{
		if ( ItemDetectSphere )
		{
			ItemDetectSphere->SetSphereRadius(ItemDetectRange);
		}
	}
}



