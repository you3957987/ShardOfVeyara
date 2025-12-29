#include "Component/PetTalkComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/AudioComponent.h"
#include "HUD/ConversationSubtitle.h"
#include "HUD/TravelSubtitle.h"
#include "Kismet/GameplayStatics.h"
#include "Ping/PingActor.h"

UPetTalkComponent::UPetTalkComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UPetTalkComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( TravelSubtitleClass )
	{
		TravelSubtitleInstance = CreateWidget<UTravelSubtitle>(GetWorld(), TravelSubtitleClass);
		if (TravelSubtitleInstance)
		{
			TravelSubtitleInstance->AddToViewport();
			// 처음엔 숨기기
			TravelSubtitleInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if ( ConversationSubtitleClass )
	{
		ConversationSubtitleInstance = CreateWidget<UConversationSubtitle>(GetWorld(), ConversationSubtitleClass);
		if (ConversationSubtitleInstance)
		{
			ConversationSubtitleInstance->AddToViewport();
			// 처음엔 숨기기
			ConversationSubtitleInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	BindInputForPet();
}

void UPetTalkComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UPetTalkComponent::BindInputForPet()
{
	// [추가] 입력 액션 바인딩 (한 번만 연결해두면 됩니다)
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent))
		{
			if (SkipDialogueAction)
			{
				// "Started" 타이밍에 SkipCurrentDialogue 함수 실행
				EnhancedInput->BindAction(SkipDialogueAction, ETriggerEvent::Started,
					this, &UPetTalkComponent::SkipCurrentDialogue);
			}
		}
	}
}

// 외부에서 아이템 발견시 호출 -> 랜덤하게 대사를 뽐아서 Travel_Say 호출
void UPetTalkComponent::Travel_ItemDetect(AActor* DetectedItem, const FVector& PingSpawnLocation)
{
	// 예외 처리 = 널 체크 + 이미 대사 출력한 아이템인지 확인
	if (!DetectedItem || DetectedItemHistory.Contains(DetectedItem)) return;

	SpawnItemPingEffectAtLocation( DetectedItem->GetActorLocation(), PingSpawnLocation );
	
	FPetConversationData SelectedDialogue;
	// 데이터 테이블 랜덤 함수를 통해 랜덤 데이터 가져오기
	if (GetRandomDialogueFromTable(TravelItemDetectDataTable, SelectedDialogue))
	{
		DetectedItemHistory.Add(DetectedItem);

		// 지속 시간 계산
		float Duration = 5.0f;
		
		Travel_Say(SelectedDialogue, Duration);

		// 히스토리 초기화 타이머
		if (!GetWorld()->GetTimerManager().IsTimerActive(HistoryResetTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(HistoryResetTimerHandle,
				this, &UPetTalkComponent::ResetDetectedItemHistory, 10.0f, false);
		}
	}
}

void UPetTalkComponent::SpawnItemPingEffectAtLocation(const FVector& TargetLocation, const FVector& PingSpawnLocation)
{
	if (!PingActorClass) return;

	// 시작 위치는 핑스폰 씬 컴포넌트 위치
	FVector StartLocation = PingSpawnLocation;

	// 핑 액터 스폰
	APingActor* PingActor = GetWorld()->SpawnActor<APingActor>(
		PingActorClass,
		StartLocation,
		FRotator::ZeroRotator
	);

	if (PingActor)
	{
		// 목표 지점으로 이동 시작 (PingActor에 구현된 함수 호출)
		PingActor->StartPingMovement(TargetLocation);
	}
}

void UPetTalkComponent::Travel_FollowToBattle()
{
	FPetConversationData SelectedDialogue;
	if (GetRandomDialogueFromTable(Travel_FollowToBattleDialogue, SelectedDialogue))
	{
		float Duration = (SelectedDialogue.VoiceAudio) ? SelectedDialogue.VoiceAudio->GetDuration() : 3.0f;
		if (Duration <= 0.0f) Duration = 3.0f;

		Travel_Say(SelectedDialogue, Duration);
	}
}

void UPetTalkComponent::Travel_BattleToFollow()
{
	FPetConversationData SelectedDialogue;
	if (GetRandomDialogueFromTable(Travel_BattleToFollowDialogue, SelectedDialogue))
	{
		float Duration = (SelectedDialogue.VoiceAudio) ? SelectedDialogue.VoiceAudio->GetDuration() : 3.0f;
		if (Duration <= 0.0f) Duration = 3.0f;

		Travel_Say(SelectedDialogue, Duration);
	}
}

void UPetTalkComponent::ResetDetectedItemHistory()
{
	DetectedItemHistory.Empty();
}

// 탐험시 화면 좌하단에 대사 출력
void UPetTalkComponent::Travel_Say(FPetConversationData DialogueData, float Duration)
{
	// 1. 현재 음성이 재생 중이라면 새로운 탐험 대사(아이템 감지, 전투 전환 등)는 무시
	if (CurrentFollowVoiceAudioComponent && CurrentFollowVoiceAudioComponent->IsPlaying()) return;
	
	// 2. 자막 표시
	if (TravelSubtitleInstance)
	{
		TravelSubtitleInstance->ShowSubtitle(DialogueData.DialogueText, Duration, DialogueData.PetIcon);
	}
	
	// 3. 음성 재생
	if (DialogueData.VoiceAudio)
	{
		// SpawnSound2D를 사용해야 IsPlaying()으로 체크가 가능합니다.
		CurrentFollowVoiceAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), DialogueData.VoiceAudio);
	}
}

void UPetTalkComponent::StartConversation(FName DialogueID)
{
    //  데이터 테이블 유효성 체크
    if (!BigConversationDataTable || DialogueID.IsNone())
    {
        EndConversation(); // ID가 없으면 종료
        return;
    }

	// [컨텍스트 교체] 기본 -> 대화용
	if (!bIsInputBound)
	{
		bIsInputBound = true; // [Tick 활성화

		// [컨텍스트 교체] 기본 -> 대화용
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PC->SetIgnoreMoveInput(true);

			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
					LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
				{
                    
					// 2. 대화용 컨텍스트를 높은 우선순위(10)로 추가
					// 주의: 이 컨텍스트가 비어있으면 스페이스바 입력이 기본 컨텍스트로 넘어가서 점프가 발생함
					if (ConversationMappingContext)
					{
						Subsystem->AddMappingContext(ConversationMappingContext, 10);
					}
				}
			}
		}
	}

    //  데이터 테이블에서 ID로 행(Row) 검색
    static const FString ContextString(TEXT("StartConversation_Context"));
    FPetConversationData* RowData = BigConversationDataTable->FindRow<FPetConversationData>(DialogueID, ContextString);

    if (RowData)
    {
        //  UI 업데이트
        if (ConversationSubtitleInstance)
        {
            // 대화창이 꺼져있다면 페이드 인 (연속 대화 중에는 깜빡이지 않게 처리)
            if (!ConversationSubtitleInstance->IsVisible())
            {
                ConversationSubtitleInstance->PlayFadeInAnimation();
            }
            // 텍스트 갱신
            ConversationSubtitleInstance->SetConversationSubtitle(RowData->SpeakerName, RowData->DialogueText);
        }

    	//  음성 재생 및 지속 시간 계산
    	float Duration = 7.0f;
    	if (RowData->VoiceAudio)
    	{
    		//  기존 음성 중지 로직 추가
    		if (CurrentConversationVoiceAudioComponent && CurrentConversationVoiceAudioComponent->IsPlaying())
    		{
    			CurrentConversationVoiceAudioComponent->Stop();
    		}

    		// [수정] SpawnSound2D로 변경하여 제어권 획득
    		CurrentConversationVoiceAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), RowData->VoiceAudio);
			
    		Duration = RowData->VoiceAudio->GetDuration();
    	}
        // 안전 장치: 너무 짧으면 7초로 고정
        if (Duration <= 0.0f) Duration = 7.0f;

        //  다음 대사 예약 (체이닝 로직)
    	
    	NextDialogueID = RowData->NextDialogueID;
    	
        FTimerDelegate TimerDel;
        if (!NextDialogueID.IsNone() && NextDialogueID != FName("0")) // 다음 대화 ID가 있으면
        {
            // 다음 대화 ID가 있으면: Duration 후에 StartConversation을 다시 호출 (재귀)
            TimerDel.BindUObject(this, &UPetTalkComponent::StartConversation, NextDialogueID);
        }
        else
        {
            // 다음 대화 ID가 없으면(None): Duration 후에 대화 종료
            TimerDel.BindUObject(this, &UPetTalkComponent::EndConversation);
        }

        // 기존 타이머 초기화 후 새로 설정
        GetWorld()->GetTimerManager().ClearTimer(ConversationTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(ConversationTimerHandle, TimerDel, Duration, false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue ID '%s' not found in DataTable."), *DialogueID.ToString());
        EndConversation();
    }
}

void UPetTalkComponent::Travel_StartSmallConversation(FName DialogueID)
{
    // 1. 데이터 테이블 유효성 체크
    UDataTable* TargetTable = TravelSmallConversationDataTable ? TravelSmallConversationDataTable : BigConversationDataTable;
    if (!TargetTable || DialogueID.IsNone() || DialogueID == FName("0")) return;
    
    static const FString ContextString(TEXT("TravelSmallConversation_Context"));
    FPetConversationData* RowData = TargetTable->FindRow<FPetConversationData>(DialogueID, ContextString);

    if (RowData)
    {
        //  연속 대화를 위해 기존 음성이 있다면 중지시키고 진행
        // 이렇게 해야 IsPlaying() 가드에 걸리지 않고 다음 대사가 재생됩니다.
        if (CurrentFollowVoiceAudioComponent && CurrentFollowVoiceAudioComponent->IsPlaying())
        {
            CurrentFollowVoiceAudioComponent->Stop();
        }

        // 2. 지속 시간 계산 (보이스 우선, 없으면 5초)
        float Duration = 5.0f;
        if (RowData->VoiceAudio)
        {
            Duration = RowData->VoiceAudio->GetDuration();
        }
        if (Duration <= 0.0f) Duration = 5.0f;

        // 3. 자막 표시 및 음성 재생 (준혁님이 요청하신 직접 구현 방식)
        if (TravelSubtitleInstance)
        {
            TravelSubtitleInstance->ShowSubtitle(RowData->DialogueText, Duration, RowData->PetIcon);
        }

        if (RowData->VoiceAudio)
        {
            CurrentFollowVoiceAudioComponent = UGameplayStatics::SpawnSound2D(GetWorld(), RowData->VoiceAudio);
        }

        // 4. 다음 대사 예약 (체이닝)
        if (!RowData->NextDialogueID.IsNone() && RowData->NextDialogueID != FName("0"))
        {
            FTimerDelegate TimerDel;
            TimerDel.BindUObject(this, &UPetTalkComponent::Travel_StartSmallConversation, RowData->NextDialogueID);

            // [수정 포인트 2] 대사 사이에 아주 짧은 간격(0.1~0.2초)을 주면 훨씬 자연스럽습니다.
            float DelayBetweenLines = Duration + 0.1f; 

            GetWorld()->GetTimerManager().ClearTimer(TravelSmallConversationTimerHandle);
            GetWorld()->GetTimerManager().SetTimer(TravelSmallConversationTimerHandle, TimerDel, DelayBetweenLines, false);
        }
    }
}

void UPetTalkComponent::SkipCurrentDialogue()
{
	// 대화 타이머가 돌고 있지 않다면(대화 중이 아님) 무시
	if (!GetWorld()->GetTimerManager().IsTimerActive(ConversationTimerHandle))
	{
		return;
	}

	//  현재 진행 중인 타이머 강제 종료
	GetWorld()->GetTimerManager().ClearTimer(ConversationTimerHandle);

	//  현재 재생 중인 음성 즉시 중지
	if (CurrentConversationVoiceAudioComponent && CurrentConversationVoiceAudioComponent->IsPlaying())
	{
		CurrentConversationVoiceAudioComponent->Stop();
	}

	//  즉시 다음 로직 실행
	// 다음 대화가 있으면 바로 시작, 없으면 종료
	if (!NextDialogueID.IsNone() && NextDialogueID != FName("0"))
	{
		StartConversation(NextDialogueID);
	}
	else
	{
		EndConversation();
	}
}

void UPetTalkComponent::EndConversation()
{
	//  UI 페이드 아웃 처리
	if (ConversationSubtitleInstance)
	{
		ConversationSubtitleInstance->PlayFadeOutAnimation();
	}

	// [추가] 대화 상태 플래그 해제 (Tick의 스킵 로직 중지)
	bIsInputBound = false;

	// [컨텍스트 복구] 대화용 -> 기본
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				// 1. 대화용 컨텍스트 제거
				if (ConversationMappingContext)
				{
					Subsystem->RemoveMappingContext(ConversationMappingContext);
				}
				
			}
		}
		// 이동 입력 차단 해제
		PC->SetIgnoreMoveInput(false);
	}
	
	// 이 컴포넌트를 가진 액터가 이 이벤트를 (Bind)하면 실행됨
	if (OnConversationEnded.IsBound())
	{
		OnConversationEnded.Broadcast();
	}
}

// 데이터 테이블에서 랜덤한 행 데이터 OutData 반환
bool UPetTalkComponent::GetRandomDialogueFromTable(UDataTable* DataTable, FPetConversationData& OutData)
{
	if (!DataTable) return false;

	TArray<FName> RowNames = DataTable->GetRowNames();
	if (RowNames.Num() == 0) return false;

	int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
	FName SelectedRowName = RowNames[RandomIndex];

	static const FString ContextString(TEXT("GetRandomDialogue_Context"));
	FPetConversationData* RowData = DataTable->FindRow<FPetConversationData>(SelectedRowName, ContextString);

	if (RowData)
	{
		OutData = *RowData; // 구조체 복사
		return true;
	}
	return false;
}

