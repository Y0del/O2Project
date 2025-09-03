#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EX_BlueprintFunctionLibrary.h"
#include "DocentTrigger.generated.h"

class UBoxComponent;
class UTextRenderComponent;

/**
 * �÷��̾ Box�� �������ϸ� JSON���� ����Ʈ �ؽ�Ʈ�� �о�
 * HoloAnchor ��ġ�� TextRenderComponent�� �״�� �ѷ��ش�.
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

    /** ���� ���ڿ� ����� + ǥ��/���� */
    void ShowWithInfo(const FDocentInfo& Info);
    void HideText();

    /** �� ���� �ڵ� �ٹٲ� */
    FString WrapText(const FString& In, int32 InMaxCharsPerLine) const;

private:
    /** ��Ʈ */
    UPROPERTY(VisibleAnywhere, Category = "Holo")
    USceneComponent* Root;

    /** ������ ���� */
    UPROPERTY(VisibleAnywhere, Category = "Holo")
    UBoxComponent* Box;

    /** ǥ�� ��ġ/���� ������(�����Ϳ��� �̵�/ȸ��) */
    UPROPERTY(VisibleAnywhere, Category = "Holo")
    USceneComponent* HoloAnchor;

    /** ���� ���� ǥ�ÿ� */
    UPROPERTY(VisibleAnywhere, Category = "Holo|Text")
    UTextRenderComponent* TextRender;

public:
    /** JSON ��ġ(���� �Ǵ� catalog.json URL/����) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Docent|Data")
    FString BaseURL;

    /** ã�� ��ǰ ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Docent|Data")
    FString ArtworkId;

    /** ��� �ڵ� ("ko","en" ��) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Docent|Data")
    FString Lang = TEXT("ko");

    /** ������ ����� �ڵ� ���� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Behavior")
    bool bAutoHideOnEndOverlap = true;

    /** �� �� �ε� �� ���û �� ��(ĳ��) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Behavior")
    bool bRequestOnce = true;

    /** ȭ�� �ɼ� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text")
    FColor TextColor = FColor::White;

    /** �� ���� ũ��(�Ÿ� ���) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text", meta = (ClampMin = "1.0"))
    float WorldSize = 32.f;

    /** �ڵ� �ٹٲ� ���� ���� ��(0�̸� �ٹٲ� ����) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text", meta = (ClampMin = "0"))
    int32 MaxCharsPerLine = 30;

    /** �¿� ����(ù ����ó�� �Ųٷ� ���̰�) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text")
    bool bMirrorText = true;

    /** �ѱ� ��Ʈ�� �ʿ��ϸ� ���� ����(������ �⺻ ��Ʈ) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Holo|Text")
    TObjectPtr<class UFont> Font = nullptr;

private:
    bool bAlreadyRequested = false;
};
