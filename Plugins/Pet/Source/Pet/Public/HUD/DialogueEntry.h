#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueEntry.generated.h"

UCLASS()
class PET_API UDialogueEntry : public UUserWidget
{
	GENERATED_BODY()


protected:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SpeakerName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DialogueText;


public:
	void SetLogData(const FText& InSpeakerName, const FText& InDialogueText);
};
