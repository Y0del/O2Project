#include "DocentTrigger.h"

#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ADocentTrigger::ADocentTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    // 오버랩 박스
    Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
    Box->SetupAttachment(Root);
    Box->InitBoxExtent(FVector(120.f));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionProfileName(TEXT("Trigger"));
    Box->SetGenerateOverlapEvents(true);

    // 고정 앵커
    HoloAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("HoloAnchor"));
    HoloAnchor->SetupAttachment(Root);

    // 월드 텍스트
    TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
    TextRender->SetupAttachment(HoloAnchor);
    TextRender->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    TextRender->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    TextRender->SetTextRenderColor(TextColor);
    TextRender->SetWorldSize(WorldSize);
    TextRender->SetVisibility(false);     // 처음엔 숨김
    TextRender->SetText(FText::FromString(TEXT("")));
}

void ADocentTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (Box)
    {
        Box->OnComponentBeginOverlap.AddDynamic(this, &ADocentTrigger::OnBoxBeginOverlap);
        Box->OnComponentEndOverlap.AddDynamic(this, &ADocentTrigger::OnBoxEndOverlap);
    }

    // 폰트 지정(선택)
    if (Font)
    {
        TextRender->Font = Font;
    }

    // 좌우반전 원하면 X스케일 -1
    if (bMirrorText)
    {
        FVector S = TextRender->GetRelativeScale3D();
        S.X = -FMath::Abs(S.X);
        TextRender->SetRelativeScale3D(S);
    }
}

void ADocentTrigger::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn || !Pawn->IsPlayerControlled()) return;

    if (bRequestOnce && bAlreadyRequested)
    {
        if (TextRender) TextRender->SetVisibility(true);
        return;
    }

    if (!BaseURL.IsEmpty() && !ArtworkId.IsEmpty())
    {
        bAlreadyRequested = true;

        FOnDocentFetched Callback;
        Callback.BindUFunction(this, FName(TEXT("HandleDocentFetched")));
        UEX_BlueprintFunctionLibrary::FetchDocentForArtwork(BaseURL, ArtworkId, Lang, Callback);
    }
    else
    {
        if (TextRender) TextRender->SetVisibility(true);
    }
}

void ADocentTrigger::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bAutoHideOnEndOverlap) return;

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (Pawn && Pawn->IsPlayerControlled())
    {
        HideText();
    }
}

void ADocentTrigger::HandleDocentFetched(bool bSuccess, int32 StatusCode, const FDocentInfo& Info)
{
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DocentTrigger] Fetch failed (code=%d) for %s"),
            StatusCode, *ArtworkId);
        if (TextRender) TextRender->SetVisibility(true);
        return;
    }

    ShowWithInfo(Info);
}

FString ADocentTrigger::WrapText(const FString& In, int32 InMaxCharsPerLine) const
{
    if (InMaxCharsPerLine <= 0) return In;

    FString Out; Out.Reserve(In.Len() + In.Len() / MaxCharsPerLine);
    int32 Count = 0;

    for (int32 i = 0; i < In.Len(); ++i)
    {
        const TCHAR C = In[i];
        Out.AppendChar(C);
        // \n이면 카운트 리셋
        if (C == TEXT('\n'))
        {
            Count = 0;
            continue;
        }

        Count++;

        // 공백에서 줄바꿈 시도
        if (Count >= InMaxCharsPerLine)
        {
            // 직전 공백이면 거기서 줄바꿈
            if (i + 1 < In.Len() && In[i + 1] == TEXT(' '))
            {
                Out.AppendChar(TEXT('\n'));
                ++i; // 공백 하나 소비
                Count = 0;
            }
            else
            {
                Out.AppendChar(TEXT('\n'));
                Count = 0;
            }
        }
    }
    return Out;
}

void ADocentTrigger::ShowWithInfo(const FDocentInfo& Info)
{
    if (!TextRender) return;

    FString Title = Info.Title;
    FString Line2 = Info.Artist.IsEmpty() && Info.Year == 0
        ? FString()
        : FString::Printf(TEXT("%s%s%s"),
            *Info.Artist,
            (Info.Artist.IsEmpty() || Info.Year == 0) ? TEXT("") : TEXT(" "),
            (Info.Year == 0) ? TEXT("") : *FString::Printf(TEXT("(%d)"), Info.Year));

    FString Desc = Info.Description;
    if (MaxCharsPerLine > 0)
    {
        Desc = WrapText(Desc, MaxCharsPerLine);
    }

    FString All;
    if (Line2.IsEmpty())
    {
        All = FString::Printf(TEXT("%s\n\n%s"), *Title, *Desc);
    }
    else
    {
        All = FString::Printf(TEXT("%s\n%s\n\n%s"), *Title, *Line2, *Desc);
    }

    // 배치/표시
    TextRender->SetTextRenderColor(TextColor);
    TextRender->SetWorldSize(WorldSize);
    TextRender->SetText(FText::FromString(All));

    TextRender->SetWorldLocation(HoloAnchor->GetComponentLocation());
    TextRender->SetWorldRotation(HoloAnchor->GetComponentRotation());
    TextRender->SetVisibility(true);
}

void ADocentTrigger::HideText()
{
    if (TextRender)
    {
        TextRender->SetVisibility(false);
    }
}
