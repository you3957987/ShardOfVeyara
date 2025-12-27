// PetConversationInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PetConversationInterface.generated.h"

// 언리얼 엔진 리플렉션용 클래스 (내용 없음)
UINTERFACE(MinimalAPI)
class UPetConversationInterface : public UInterface
{
	GENERATED_BODY()
};

// 실제 기능을 정의하는 인터페이스 클래스
class PET_API IPetConversationInterface
{
	GENERATED_BODY()

public:
	// 대화 시작을 요청하는 함수 (BlueprintNativeEvent로 선언하여 C++과 블루프린트 양쪽에서 구현 가능하게 함)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Conversation")
	void TriggerPetConversation(FName DialogueID);
};
