#include "BaseFlyingPet.h"

#include "Component/PetTalkComponent.h"
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

	// 아이템 감지 핑 생성 위치 컴포넌트
	ItemDetectPingSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ItemDetectPingSpawnPoint"));
	ItemDetectPingSpawnPoint->SetupAttachment(RootComponent);
	
	// 펫 대화 컴포넌트 생성
	PetTalkComp = CreateDefaultSubobject<UPetTalkComponent>(TEXT("PetTalkComp"));
	
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
	if (PetTalkComp)
	{
		// 대화가 끝나면 내 클래스의 EndConversation 함수를 실행해라! 라고 등록
		PetTalkComp->OnConversationEnded.AddDynamic(this, &ABaseFlyingPet::EndConversation);
	}

	// [추가] 외부에서 OnPetConversationStart를 Broadcast하면 자동으로 StartConversation이 실행되도록 연결
	OnPetConversationStart.AddDynamic(this, &ABaseFlyingPet::StartConversation);
	
	// 일정 주기마다 주변 적 감지 함수 호출 설정
	GetWorld()->GetTimerManager().SetTimer(
		EnemyDetectTimerHandle, 
		this, 
		&ABaseFlyingPet::CheckSurroundingEnemy, 
		EnemyDetectInterval, 
		true // 반복 여부: true
	);
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

	// 상태(PetState)에 따른 설정값 선택 
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

	// [수정] 거리에 따른 선형 가속 적용
	float DistanceToTarget = FVector::Dist(CurrentLocation, DesiredLocation);
	
	// 거리가 멀수록 이동 속도 선형 증가 (예: 1000단위로 5% 증가 등)
	float MoveSpeedMultiplier = 1.0f + (DistanceToTarget * 0.005f);
	float FinalInterpSpeed = MoveInterpSpeed * MoveSpeedMultiplier;
	
	// 위치 보간 (부드럽게 이동) - 수정된 FinalInterpSpeed 사용
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaTime, FinalInterpSpeed);
	SetActorLocation(NewLocation);
	
	if (DeltaTime > KINDA_SMALL_NUMBER)
	{
		// (이동 후 위치 - 이동 전 위치) / 시간 = 속도 -> ABP에 전달할 현재 속도 계산
		CurrentVelocity = (NewLocation - CurrentLocation) / DeltaTime;
	}
	else
	{
		CurrentVelocity = FVector::ZeroVector;
	}
	
	// --- 2. 회전 계산 (상태에 따른 분기) ---
	FRotator TargetRotation;

	if (PetState == EPetState::EPS_Conversation)
	{
		// [대화 상태] 펫이 캐릭터를 바라보도록 회전 계산 (LookAt)
		TargetRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
	}
	else
	{
		TargetRotation = TargetActor->GetActorRotation();
	}

	// [수정] 회전 속도도 각도 차이에 따라 선형 증가
	FRotator CurrentRotation = GetActorRotation();
	float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

	// 각도 차이가 클수록 회전 속도 선형 증가 (예: 10도 차이날 때마다 0.5배씩 증가 등)
	float RotSpeedMultiplier = 1.0f + (DeltaYaw * 0.05f);
	float FinalRotationSpeed = MoveInterpSpeed * RotSpeedMultiplier;

	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, FinalRotationSpeed);
	SetActorRotation(NewRotation);
}

void ABaseFlyingPet::CheckSurroundingEnemy()
{
	// 현재 상태가 대화 중이면 적 감지 무시
	if (!EnemyDetectSphere || PetState == EPetState::EPS_Conversation) return;

	TArray<AActor*> OverlappingActors;
	// 현재 스피어 안에 있는 모든 액터를 가져옵니다. (필터링할 클래스가 있다면 두 번째 인자에 넣음)
	EnemyDetectSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	bool bEnemyFound = false;

	for (AActor* Actor : OverlappingActors)
	{
		// 나 자신은 제외
		if (Actor == this) continue;

		// 적 태그 확인 (혹은 인터페이스나 클래스 캐스팅 확인)
		if (Actor->ActorHasTag("Enemy"))
		{
			// Boss 태그가 있다면 무시 
			if (Actor->ActorHasTag("Boss")) continue;
			
			bEnemyFound = true;
			break; // 한 명이라도 있으면 배틀 모드이므로 더 검사할 필요 없음
		}
	}

	// 상태 업데이트
	if (bEnemyFound)
	{
		if (PetState != EPetState::EPS_Battle)
		{
			// 평상시 상태였는데 주변 적이 있다는 의미
			PetState = EPetState::EPS_Battle;
			if ( PetTalkComp ) PetTalkComp->Travel_FollowToBattle();
		}
	}
	else
	{
		if (PetState == EPetState::EPS_Battle) 
		{
			// 배틀 모드에서 벗어나 평상시 상태로 복귀
			PetState = EPetState::EPS_Follow;
			if ( PetTalkComp ) PetTalkComp->Travel_BattleToFollow();
		}
	}
}

void ABaseFlyingPet::OnItemDetectBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 통상적인 따라다니기 상태가 아닐 경우 무시 -> 배틀 모드 등에서 아이템 반응 안하도록
	if ( PetState != EPetState::EPS_Follow ) return;
	
	// 겹친 액터가 Item 태그를 가지고 있는지 확인
	if ( OtherActor && OtherActor != this && OtherActor->ActorHasTag("Item") )
	{
		// 하나의 아이템에 대해 여러 콜리전 컴포넌트가 있을 수 있으므로, 하나만 처리하도록 루트 컴포넌트 확인
		if ( OtherActor->GetRootComponent() != OtherComp ) return;

		// 아이템 획득 로직 구현 (예: 아이템 파괴)
		UE_LOG(LogTemp, Warning, TEXT("Pet Item Detected: %s"), *OtherActor->GetName());

		if ( PetTalkComp ) PetTalkComp->Travel_ItemDetect(OtherActor, ItemDetectPingSpawnPoint->GetComponentLocation());
	}
}

void ABaseFlyingPet::TriggerPetConversation_Implementation(FName DialogueID)
{
	StartConversation(DialogueID);
}

void ABaseFlyingPet::StartConversation( FName DialogueID )
{
	// 대화 상태로 전환
	PetState = EPetState::EPS_Conversation;

	// 2. 대화 컴포넌트에 실제 대화 시작 요청
	if (PetTalkComp)
	{
		PetTalkComp->StartConversation(DialogueID);
	}
}

void ABaseFlyingPet::EndConversation()
{
	// 따라다니기 상태로 복귀
	PetState = EPetState::EPS_Follow;
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



