// Fill out your copyright notice in the Description page of Project Settings.


#include "AGSDGameStateBase.h"
#include "Net/UnrealNetwork.h"

void AAGSDGameStateBase::OnRep_CurrentDay()
{
	UE_LOG(LogTemp, Warning, TEXT("Day %d"), CurrentDay);
}

void AAGSDGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAGSDGameStateBase, CurrentDay);
	DOREPLIFETIME(AAGSDGameStateBase, CurrentTime);
}

void AAGSDGameStateBase::AdvanceDay()
{
	// 이 함수는 서버에서만 호출됩니다.
	if (!HasAuthority()) return; 

	CurrentDay++;

	// 💡 날짜가 바뀌었음을 모든 구독자에게 알립니다.
	OnDayChangedDelegate.Broadcast(CurrentDay);
    
	// (선택 사항: 디버깅 로그)
	UE_LOG(LogTemp, Warning, TEXT("Game Day Advanced to Day %d!"), CurrentDay);
}
