#include "TextLog.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"

void UTextLog::WriteTextLogByKeyword(const FString& Keyword)
{
	WriteTextLogByString(Keyword, TEXT(""));
}

void UTextLog::WriteTextLogByString(const FString& Keyword, const FString& Value)
{
	const FString Directory = FPaths::ProjectSavedDir() / TEXT("TextLogs");
	const FString FilePath = Directory / TEXT("PlayDataLog.txt");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*Directory)) // * 제거하여 표준 안전성 확보
	{
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	FString FinalLine;
    
	// Value가 비어있다면 콜론(:) 없이 깔끔하게 출력
	if (Value.IsEmpty())
	{
		FinalLine = FString::Printf(TEXT("[%s] [%s]%s"), 
		   *FDateTime::Now().ToString(), 
		   *Keyword, 
		   LINE_TERMINATOR);
	}
	else
	{
		FinalLine = FString::Printf(TEXT("[%s] [%s] : %s%s"), 
		   *FDateTime::Now().ToString(), 
		   *Keyword, 
		   *Value, 
		   LINE_TERMINATOR);
	}

	FFileHelper::SaveStringToFile(
	   FinalLine, 
	   *FilePath, 
	   FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, 
	   &IFileManager::Get(), 
	   FILEWRITE_Append
	);
}

void UTextLog::WriteTextLogByFloat(const FString& Keyword, float Value)
{
	// %.2f 를 사용하여 소수점 둘째 자리까지 표기합니다. (자동 반올림 적용)
	FString ConvertedValue = FString::Printf(TEXT("%.2f"), Value);
    
	// 이미 잘 만들어진 String 버전 함수로 토스해서 파일에 기록
	WriteTextLogByString(Keyword, ConvertedValue);
}

void UTextLog::WriteTextLogByStringAndFloat(const FString& Keyword, const FString& StringValue, float FloatValue)
{
	// %.2f 를 사용하여 소수점 둘째 자리까지 표기합니다. (자동 반올림 적용)
	FString ConvertedFloatValue = FString::Printf(TEXT("%.2f"), FloatValue);
	FString CombinedValue = StringValue + TEXT(" | ") + ConvertedFloatValue;

	WriteTextLogByString(Keyword, CombinedValue);
}

