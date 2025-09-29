#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TTSAudioHelper.generated.h"

UCLASS()
class EXHIBITION_API UTTSAudioHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "TTS")
    static bool SaveBase64ToFile(const FString& Base64String, const FString& FileName, FString& OutFullPath);

    UFUNCTION(BlueprintCallable, Category = "TTS")
    static FString MakeFileUrlFromPath(const FString& FullPath);
};