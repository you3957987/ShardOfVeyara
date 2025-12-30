#include "HUD/TravelSubtitle.h"
#include "Components/TextBlock.h" 
#include "TimerManager.h" 
#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"

void UTravelSubtitle::ShowSubtitle(FText InText, float Duration, UTexture2D* InPetIcon)
{
	
	if (Text_Subtitle)
	{
		Text_Subtitle->SetText(InText);
	}

	// 2. 펫 아이콘 이미지 설정 (이미지가 들어왔을 때만 변경)
	if (Image_PetIcon && InPetIcon)
	{
		Image_PetIcon->SetBrushFromTexture(InPetIcon);
	}
	
	// 1. 보이게 설정
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 2. FadeIn 재생
	if (FadeInAnim)
	{
		PlayAnimation(FadeInAnim);
	}

	// 3. 기존 타이머가 있다면 초기화 (Retriggerable Delay 효과)
	if (GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DisplayTimerHandle);
	}
	
	// FadeOut 중이었다면 멈추고 다시 선명하게
	if (IsAnimationPlaying(FadeOutAnim))
	{
		StopAnimation(FadeOutAnim);
		// 필요하다면 Opacity를 1로 강제 설정하는 로직 추가 필요
	}

	// 4. Duration 후에 FadeOut 시작하도록 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(DisplayTimerHandle, this, &UTravelSubtitle::StartFadeOut, Duration, false);
}

void UTravelSubtitle::StartFadeOut()
{
	if (FadeOutAnim)
	{
		PlayAnimation(FadeOutAnim);

		// FadeOut 애니메이션 길이만큼 기다렸다가 완전히 숨김 (예: 0.5초)
		float AnimEndTime = FadeOutAnim->GetEndTime();
		GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &UTravelSubtitle::OnFadeOutFinished, AnimEndTime, false);
	}
	else
	{
		// 애니메이션 없으면 바로 숨김
		OnFadeOutFinished();
	}
}

void UTravelSubtitle::OnFadeOutFinished()
{
	SetVisibility(ESlateVisibility::Hidden);
}
