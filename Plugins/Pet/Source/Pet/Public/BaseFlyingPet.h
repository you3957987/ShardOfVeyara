#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
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
	EPS_Conversation UMETA(DisplayName = "Conversation"),
	EPS_Battle UMETA(DisplayName = "Battle"),
	
	EPS_MAX UMETA(DisplayName = "Default") // 최대값, 추가적인 값을 위한 공간
};

UCLASS()
class PET_API ABaseFlyingPet : public APawn
{
	GENERATED_BODY()

	// 캐릭터 초기화 여부를 나타내는 변수
	bool bTargetInitalize = false;
	
protected:
	virtual void BeginPlay() override;

	void PollInit(float DeltaTime); // 틱에서 하는 초기화

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
	// 아이템 감지 스피어에 아이템 겹침 이벤트 처리 함수
	UFUNCTION()
	void OnItemDetectBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	

	// 공중에서 떠다니는 움직임을 위한 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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

	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bDebugMode = false;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float EnemyDetectRange = 500.f;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ItemDetectRange = 300.f;
	
	void FollowingTarget(float DeltaTime);
	
public:
	ABaseFlyingPet();
	virtual void Tick(float DeltaTime) override;


#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
