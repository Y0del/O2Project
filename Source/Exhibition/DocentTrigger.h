#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EX_BlueprintFunctionLibrary.h"
#include "DocentTrigger.generated.h"

class UBoxComponent;
class UTextRenderComponent;

/**
 * 플레이어가 Box와 오버랩하면 JSON에서 도슨트 텍스트를 읽어
 * HoloAnchor 위치에 TextRenderComponent로 그대로 뿌려준다.
 */
UCLASS()
class EXHIBITION_API ADocentTrigger : public AActor
{
    GENERATED_BODY()

public:
    ADocentTrigger();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void HandleDocentFetched(bool bSuccess, int32 StatusCode, const FDocentInfo& Info);

    /** 최종 문자열 만들기 + 표시/숨김 */
    void ShowWithInfo(const FDocentInfo& Info);
    void HideText();

    /** 긴 설명 자동 줄바꿈 */
    FString WrapText(const FString& In, int32 InMaxCharsPerLine) const;

private:
    /** 루트 */
    UPROPERTY(VisibleAnywhere, Category = "Holo")
    USceneComponent* Root;

    /** 오버랩 감지 */
    UPROPERTY(VisibleAnywhere, Category = "Holo")
    UBoxComponent* Box;

    /** 표시 위치/각도 기준점(에디터에서 이동/회전) */
    UPROPERTY(VisibleAnywhere, Category = "Holo")
    USceneComponent* HoloAnchor;

    /** 실제 글자 표시용 */
    UPROPERTY(VisibleAnywhere, Category = "Holo|Text")
    UTextRenderComponent* TextRender;

public:
    /** JSON 위치(폴더 또는 catalog.json URL/파일) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Docent|Data")
    FString BaseURL;

    /** 찾을 작품 ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Docent|Data")
    FString ArtworkId;

    /** 언어 코드 ("ko","en" 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Docent|Data")
    FString Lang = TEXT("ko");

    /** 범위를 벗어나면 자동 숨김 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Behavior")
    bool bAutoHideOnEndOverlap = true;

    /** 한 번 로드 후 재요청 안 함(캐시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Behavior")
    bool bRequestOnce = true;

    /** 화면 옵션 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text")
    FColor TextColor = FColor::White;

    /** 한 글자 크기(거리 비례) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text", meta = (ClampMin = "1.0"))
    float WorldSize = 32.f;

    /** 자동 줄바꿈 기준 문자 수(0이면 줄바꿈 안함) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text", meta = (ClampMin = "0"))
    int32 MaxCharsPerLine = 30;

    /** 좌우 반전(첫 스샷처럼 거꾸로 보이게) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text")
    bool bMirrorText = true;

    /** 한글 폰트가 필요하면 여기 지정(없으면 기본 폰트) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text")
    TObjectPtr<class UFont> Font = nullptr;

private:
    bool bAlreadyRequested = false;
};
