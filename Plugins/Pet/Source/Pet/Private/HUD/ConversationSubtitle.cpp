#include "HUD/ConversationSubtitle.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"

void UConversationSubtitle::SetConversationSubtitle(const FText& InName, const FText& InDialogue)
{
	if (Text_Name)
	{
		// [수정] 이미 FText이므로 FromString 없이 바로 넣습니다.
		Text_Name->SetText(InName);
	}

	if (Text_Dialogue)
	{
		Text_Dialogue->SetText(InDialogue);
	}
}

void UConversationSubtitle::PlayFadeInAnimation()
{
	//보이게 설정
	SetVisibility(ESlateVisibility::HitTestInvisible);
	
	if (FadeInAnim)
	{
		PlayAnimation(FadeInAnim);
	}
}

void UConversationSubtitle::PlayFadeOutAnimation()
{
	if (FadeOutAnim)
	{
		PlayAnimation(FadeOutAnim);

		// FadeOut 애니메이션 길이만큼 기다렸다가 완전히 숨김 (예: 0.5초)
		float AnimEndTime = FadeOutAnim->GetEndTime();
		GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &UConversationSubtitle::OnFadeOutFinished,
			AnimEndTime, false);
	}
	else
	{
		// 애니메이션 없으면 바로 숨김
		OnFadeOutFinished();
	}
}

void UConversationSubtitle::OnFadeOutFinished()
{
	SetVisibility(ESlateVisibility::Hidden);
}
