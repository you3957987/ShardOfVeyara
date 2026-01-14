#include "HUD/DialogueEntry.h"

#include "Components/TextBlock.h"

void UDialogueEntry::SetLogData(const FText& InSpeakerName, const FText& InDialogueText)
{
	// 1. 스피커 이름 설정
	if (SpeakerName)
	{
		SpeakerName->SetText(InSpeakerName);
	}
	// 2. 대화 내용 설정
	if (DialogueText)
	{
		DialogueText->SetText(InDialogueText);
	}
}
