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
    // BaseURL: ���� �Ǵ� catalog.json ����/URL
    // ArtworkId: ã�� ID (��: "ART_002")
    // Lang: "ko","en" ��. description/title�� �ٱ��� ��ü�� �� �켱 ����
    UFUNCTION(BlueprintCallable, Category = "Docent|HTTP", meta = (AutoCreateRefTerm = "Callback"))
    static void FetchDocentForArtwork(const FString& BaseURL, const FString& ArtworkId, const FString& Lang, const FOnDocentFetched& Callback);
};
