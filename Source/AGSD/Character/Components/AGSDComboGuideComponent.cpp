#include "Character/Components/AGSDComboGuideComponent.h"
#include "Character/AGSDCharacter.h"
#include "SpearComboData.h"

UAGSDComboGuideComponent::UAGSDComboGuideComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAGSDComboGuideComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AAGSDCharacter>(GetOwner());
}

void UAGSDComboGuideComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateComboGuideUI();
}

void UAGSDComboGuideComponent::UpdateComboGuideUI()
{
	TArray<FComboGuideEntry> Entries;

	if (!OwnerCharacter || OwnerCharacter->HoldingWeapon != EHoldingWeapon::Spear || !OwnerCharacter->SpearComboDataTable)
	{
		OnComboGuideUpdated.Broadcast(Entries);
		return;
	}

	// 1. 공격 진행 중인 경우 (첫 번째 공격이 나가자마자 바로 다음 연계 콤보 안내)
	if (OwnerCharacter->bIsAttacking && OwnerCharacter->CurrentComboData)
	{
		int32 NextIndex = OwnerCharacter->CurrentStageIndex + 1;
		FText InputText = FText::FromString(TEXT("[LMB / 좌클릭]"));
		FText AttackNameText;

		if (OwnerCharacter->CurrentComboData->Stages.IsValidIndex(NextIndex))
		{
			const FSpearStageData& NextStage = OwnerCharacter->CurrentComboData->Stages[NextIndex];
			if (!NextStage.AttackName.IsEmpty())
			{
				AttackNameText = NextStage.AttackName;
			}
			else if (OwnerCharacter->CurrentComboData->DirectionRequirement == ESpearAttackDirection::Parry)
			{
				AttackNameText = FText::FromString(TEXT("카운터 공격"));
			}
			else
			{
				AttackNameText = FText::FromString(FString::Printf(TEXT("연계 찌르기 %d타"), NextIndex + 1));
			}
		}
		else
		{
			ESpearAttackDirection CurrentDir = OwnerCharacter->GetAttackDirection();
			FSpearComboData* NextComboData = OwnerCharacter->GetComboDataByDirection(CurrentDir);
			if (NextComboData && NextComboData->Stages.Num() > 0 && !NextComboData->Stages[0].AttackName.IsEmpty())
			{
				AttackNameText = NextComboData->Stages[0].AttackName;
			}
			else
			{
				AttackNameText = FText::FromString(TEXT("연계 찌르기"));
			}
		}

		Entries.Add(FComboGuideEntry(InputText, AttackNameText));
	}
	// 2. 비공격 상태 또는 평상시 가이드
	else
	{
		ESpearAttackDirection CurrentDir = OwnerCharacter->GetAttackDirection();

		// 달리기(스프린트) 상태인 경우 -> 대시 내려찍기 우선 가이드
		if (CurrentDir == ESpearAttackDirection::Sprint)
		{
			FSpearComboData* SprintCombo = OwnerCharacter->GetComboDataByDirection(ESpearAttackDirection::Sprint);
			FText SprintAttackName = (SprintCombo && SprintCombo->Stages.Num() > 0 && !SprintCombo->Stages[0].AttackName.IsEmpty())
				? SprintCombo->Stages[0].AttackName : FText::FromString(TEXT("대시 내려찍기"));
			Entries.Add(FComboGuideEntry(FText::FromString(TEXT("[Shift + LMB]")), SprintAttackName));
		}
		// 뒤로 이동(Backward / S 키) 상태인 경우 -> 후퇴 찌르기 안내
		else if (CurrentDir == ESpearAttackDirection::Backward)
		{
			FSpearComboData* BackCombo = OwnerCharacter->GetComboDataByDirection(ESpearAttackDirection::Backward);
			FText BackAttackName = (BackCombo && BackCombo->Stages.Num() > 0 && !BackCombo->Stages[0].AttackName.IsEmpty())
				? BackCombo->Stages[0].AttackName : FText::FromString(TEXT("후퇴 찌르기"));
			Entries.Add(FComboGuideEntry(FText::FromString(TEXT("[S + LMB / 후퇴]")), BackAttackName));
		}
		// 정지 또는 일반 이동 상태인 경우 -> 정지 찌르기(Neutral) 및 전진 찌르기(Forward) 모두 표시
		else
		{
			// 1st: Neutral 찌르기
			FSpearComboData* NeutralCombo = OwnerCharacter->GetComboDataByDirection(ESpearAttackDirection::Neutral);
			FText NeutralName = (NeutralCombo && NeutralCombo->Stages.Num() > 0 && !NeutralCombo->Stages[0].AttackName.IsEmpty())
				? NeutralCombo->Stages[0].AttackName : FText::FromString(TEXT("찌르기"));
			Entries.Add(FComboGuideEntry(FText::FromString(TEXT("[LMB / 좌클릭]")), NeutralName));

			// 2nd: Forward 전진 찌르기 (정지 시에도 전진 찌르기를 함께 안내)
			FSpearComboData* ForwardCombo = OwnerCharacter->GetComboDataByDirection(ESpearAttackDirection::Forward);
			FText ForwardName = (ForwardCombo && ForwardCombo->Stages.Num() > 0 && !ForwardCombo->Stages[0].AttackName.IsEmpty())
				? ForwardCombo->Stages[0].AttackName : FText::FromString(TEXT("전진 찌르기"));
			Entries.Add(FComboGuideEntry(FText::FromString(TEXT("[W + LMB / 전진]")), ForwardName));
		}
	}

	OnComboGuideUpdated.Broadcast(Entries);
}
