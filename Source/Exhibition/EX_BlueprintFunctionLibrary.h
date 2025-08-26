#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EX_BlueprintFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FDocentInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString ArtworkId;
    UPROPERTY(BlueprintReadOnly) FString Title;
    UPROPERTY(BlueprintReadOnly) FString Artist;
    UPROPERTY(BlueprintReadOnly) int32   Year = 0;
    UPROPERTY(BlueprintReadOnly) FString Description;
    UPROPERTY(BlueprintReadOnly) TArray<FString> Tags;
    UPROPERTY(BlueprintReadOnly) FString ImageURL;
    UPROPERTY(BlueprintReadOnly) FString AudioURL;
};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDocentFetched, bool, Success, int32, StatusCode, const FDocentInfo&, Info);

UCLASS()
class EXHIBITION_API UEX_BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // BaseURL: 폴더 또는 catalog.json 파일/URL
    // ArtworkId: 찾을 ID (예: "ART_002")
    // Lang: "ko","en" 등. description/title이 다국어 객체일 때 우선 선택
    UFUNCTION(BlueprintCallable, Category = "Docent|HTTP", meta = (AutoCreateRefTerm = "Callback"))
    static void FetchDocentForArtwork(const FString& BaseURL, const FString& ArtworkId, const FString& Lang, const FOnDocentFetched& Callback);
};
