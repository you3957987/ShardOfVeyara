// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyTable.h"

#include "AGSDCharacter.h"
#include "AGSDPlayerController.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAlchemyTable::AAlchemyTable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	//콜리전 박스 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAlchemyTable::OnBeginOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AAlchemyTable::OnEndOverlap);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAlchemyTable::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAlchemyTable::EndAlchemy()
{
	// 1. 플레이어 컨트롤러 가져오기
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;
    
	// 3. 카메라를 다시 캐릭터에게로 (부드럽게 복구)
	PC->SetViewTargetWithBlend(Player, BlendTime);

	// 4. 입력 모드를 다시 게임 전용으로 변경
	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = false;

	// 5. 캐릭터 상태 및 테이블 상태 복구
	if (Player)
	{
		Player->Mining = false; // 이동 가능하게 변경
	}
	bCanUseAlchemyTable = true; // 다시 상호작용 가능하게

	// 6. 위젯 제거
	if (AlchemyWidget)
	{
		AlchemyWidget->RemoveFromParent();
		// 메모리 관리를 위해 필요하다면 AlchemyWidget = nullptr; 를 해줄 수도 있지만, 
		// 다시 열 때를 대비해 유지하는 것이 일반적입니다.
	}
}

void AAlchemyTable::Interact_Implementation(AAGSDCharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
	{
		Player = player;
		Player->Mining = true;
		bCanUseAlchemyTable = false;
		
		PlayerController->SetViewTargetWithBlend(this, BlendTime);

		// 위젯이 아직 없다면 여기서 생성
		if (!AlchemyWidget && AlchemyWidgetClass)
		{
			AlchemyWidget = CreateWidget<UAlchemyUI>(GetWorld(), AlchemyWidgetClass);
		}
		if (AlchemyWidget)
		{
			AlchemyWidget->OnWidgetClosed.AddUniqueDynamic(this, &AAlchemyTable::EndAlchemy);
			
			AlchemyWidget->AddToViewport();
			
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(AlchemyWidget->TakeWidget());
			PlayerController->SetInputMode(InputMode);
			PlayerController->bShowMouseCursor = true;
		}
	}
}

void AAlchemyTable::ShowWidget_Implementation(ACharacter* player)
{
	if (AAGSDPlayerController* PlayerController = Cast<AAGSDPlayerController>(player->GetController()))
		PlayerController->ShowInteractionWidget(InteractActionText);
}

bool AAlchemyTable::CanInteract_Implementation(AAGSDCharacter* player)
{
	return bCanUseAlchemyTable;
}

void AAlchemyTable::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->AddInteractableActor(this);
	}
}

void AAlchemyTable::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AAGSDCharacter* player = Cast<AAGSDCharacter>(OtherActor))
	{
		player->RemoveInteractableActor(this);
	}
}

