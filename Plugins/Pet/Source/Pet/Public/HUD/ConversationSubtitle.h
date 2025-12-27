#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConversationSubtitle.generated.h"


UCLASS()
class PET_API UConversationSubtitle : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 캐릭터 이름을 표시하는 텍스트
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Name;

	// 대화 내용을 표시하는 텍스트 (기존 Text_Log 대체)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Dialogue;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInAnim;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnim;

public:
	// 대화 자막 설정 함수
	UFUNCTION(BlueprintCallable)
	void SetConversationSubtitle(const FText& InName, const FText& InDialogue);

	FTimerHandle FadeOutTimerHandle;
	
	UFUNCTION(BlueprintCallable)
	void PlayFadeInAnimation();
	UFUNCTION(BlueprintCallable)
	void PlayFadeOutAnimation();
	void OnFadeOutFinished();
};
