#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Interface/PetConversationInterface.h"
#include "BaseFlyingPet.generated.h"

// 펫의 위치 설정을 관리하는 구조체
USTRUCT(BlueprintType)
struct FPetPositionSettings
{
	GENERATED_BODY()

	// 앞뒤 오프셋 aka 거리 ( -는 앞쪽, +는 뒤쪽)
	UPROPERTY(EditAnywhere)
	float Distance;

	// 상하 오프셋 (-는 아래, +는 위)
	UPROPERTY(EditAnywhere)
	float UpOffset;

	// 좌우 오프셋 (-는 왼쪽, +는 오른쪽)
	UPROPERTY( EditAnywhere )
	float SideOffset;;
};

// 펫의 위치랑 매칭시킬 펫의 상태 열거형
UENUM(BlueprintType)
enum class EPetState : uint8
{
	EPS_Follow UMETA(DisplayName = "Follow"),
	EPS_Battle UMETA(DisplayName = "Battle"),
	EPS_BossBattle UMETA(DisplayName = "Boss Battle"),
	EPS_Conversation UMETA(DisplayName = "Conversation"),
	
	EPS_MAX UMETA(DisplayName = "Default") // 최대값, 추가적인 값을 위한 공간
};

// 델리게이트 선언 , 인자값은 FName DialogueID (대화 ID)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConversationStart, FName, DialogueID);

UCLASS()
class PET_API ABaseFlyingPet : public APawn, public IPetConversationInterface
{//  인터페이스 상속 추가
	GENERATED_BODY()

	friend class UPetTalkComponent; // PetTalkComponent에서 BaseFlyingPet의 protected 멤버에 접근할 수 있도록 합니다.
	
protected:
	virtual void BeginPlay() override;
	// 캐릭터 초기화 여부를 나타내는 변수
	bool bTargetInitalize = false;
	void PollInit(float DeltaTime); // 틱에서 하는 초기화
	
	// 아이템 감지 콜리전 시작 함수
	UFUNCTION()
	void OnItemDetectBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// 펫이 대상을 따라다니는 함수
	void FollowingTarget(float DeltaTime);
	
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;
	// 캐릭터 메쉬 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* MeshComp;
	// 펫이 적을 감지하는 범위 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* EnemyDetectSphere;
	// 펫이 아이템을 감지하는 범위 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* ItemDetectSphere;
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* ItemDetectPingSpawnPoint;
	// 펫의 대화 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPetTalkComponent* PetTalkComp;
	// 공중에서 떠다니는 움직임을 위한 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* FloatingMovement;

	// 따라다닐 대상 캐릭터
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "자체설정")
	ACharacter* TargetActor;
	// 펫의 현재 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	EPetState PetState;
	// 자유 이동 모드 여부 (true면 따라다니지 않음)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	bool bIsFolloingTarget;
	// 평소 따라다닐 때의 위치 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FPetPositionSettings FollowSettings;
	// 캐릭터와 대화할 때의 위치 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	FPetPositionSettings ConversationSettings;
	// 이동 속도 (보간 속도)
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MoveInterpSpeed = 1.0f;
	// 자유 이동 모드 토글
	UFUNCTION(BlueprintCallable, Category = "자체설정")
	void SetFreeRoaming(bool bNewState);
	// 디버그 모드 여부 및 감지 범위
	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bDebugMode = false;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float EnemyDetectRange = 2000.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ItemDetectRange = 1500.f;

	// 주기적으로 적을 감지하기 위한 타이머 핸들
	FTimerHandle EnemyDetectTimerHandle;
	// 주변 적 감지 함수
	void CheckSurroundingEnemy();
	//  적 감지 주기 (초 단위)
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	float EnemyDetectInterval = 0.5f;
	// 만약 보스 만나면 보스 끝나기 전까지 기능 조정용 플래그
	bool bBossBattleMode = false;
	
public:
	ABaseFlyingPet();
	virtual void Tick(float DeltaTime) override;
	// 인터페이스 함수 구현
	virtual void TriggerPetBigConversation_Implementation(FName DialogueID) override;
	virtual void TriggerPetSmallConversation_Implementation(FName DialogueID) override;
	
	// ABP 에 서 사용할 현재 속도 변수
	UPROPERTY(BlueprintReadOnly)
	FVector CurrentVelocity;
	
	// 2. 외부에서 바인딩할 델리게이트 인스턴스
	UPROPERTY(BlueprintAssignable)
	FConversationStart OnPetConversationStart;
	
	// 대화 상태로 전환하며 대화 시작 -> 펫 토크 컴포넌트로 실제 대화 로직 위임
	UFUNCTION(BlueprintCallable)
	void StartBigConversation(FName DialogueID);
	UFUNCTION(BlueprintCallable)
	void StartSmallConversation(FName DialogueID);
	// 대화 종료 및 이전 상태로 복귀 로직 처리
	UFUNCTION(BlueprintCallable)
	void EndConversation();
	
#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
