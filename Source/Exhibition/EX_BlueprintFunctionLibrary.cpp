#include "EX_BlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Http.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

static bool TryGetLocalizedText(const TSharedPtr<FJsonObject>& Obj, const FString& Field, const FString& Lang, FString& Out)
{
    if (!Obj.IsValid()) return false;

    // 1) 단일 문자열
    if (Obj->HasTypedField<EJson::String>(Field))
    {
        Out = Obj->GetStringField(Field);
        return true;
    }

    // 2) 다국어 객체 { "ko": "...", "en": "..." }
    if (Obj->HasTypedField<EJson::Object>(Field))
    {
        const TSharedPtr<FJsonObject> L = Obj->GetObjectField(Field);
        if (L->TryGetStringField(Lang, Out)) return true;
        if (L->TryGetStringField(TEXT("en"), Out)) return true;

        // 첫 번째 키 아무거나 사용
        TArray<FString> Keys;
        L->Values.GetKeys(Keys);     // ★ 인자 넘겨서 키 목록 받기
        if (Keys.Num() > 0)
        {
            Out = L->GetStringField(Keys[0]);
            return true;
        }
    }
    return false;
}

static bool ParseDocentFromJsonObject(const TSharedPtr<FJsonObject>& Obj, const FString& Lang, FDocentInfo& Out)
{
    if (!Obj.IsValid()) return false;

    Obj->TryGetStringField(TEXT("id"), Out.ArtworkId);
    Obj->TryGetStringField(TEXT("title"), Out.Title);
    Obj->TryGetStringField(TEXT("artist"), Out.Artist);
    Obj->TryGetNumberField(TEXT("year"), Out.Year);

    // title/description이 다국어 객체일 수 있음
    FString Localized;
    if (TryGetLocalizedText(Obj, TEXT("title"), Lang, Localized)) Out.Title = Localized;
    if (TryGetLocalizedText(Obj, TEXT("description"), Lang, Localized)) Out.Description = Localized;
    else Obj->TryGetStringField(TEXT("description"), Out.Description);

    Obj->TryGetStringField(TEXT("image_url"), Out.ImageURL);
    Obj->TryGetStringField(TEXT("audio_url"), Out.AudioURL);

    Out.Tags.Empty();
    const TArray<TSharedPtr<FJsonValue>>* Tags = nullptr;
    if (Obj->TryGetArrayField(TEXT("tags"), Tags))
    {
        for (const auto& V : *Tags) { FString S; if (V->TryGetString(S)) Out.Tags.Add(S); }
    }

    return !Out.ArtworkId.IsEmpty(); // 최소 id가 있어야 유효
}

static bool FindInCatalog(const FString& Json, const FString& ArtworkId, const FString& Lang, FDocentInfo& Out)
{
    // 1) 루트가 배열인 경우: [ {...}, {...} ]
    {
        TArray<TSharedPtr<FJsonValue>> Arr;
        const TSharedRef<TJsonReader<>> R = TJsonReaderFactory<>::Create(Json);
        if (FJsonSerializer::Deserialize(R, Arr) && Arr.Num() > 0)
        {
            for (const auto& V : Arr)
            {
                TSharedPtr<FJsonObject> Obj = V->AsObject();
                if (!Obj.IsValid()) continue;
                FString Id; Obj->TryGetStringField(TEXT("id"), Id);
                if (Id == ArtworkId)
                    return ParseDocentFromJsonObject(Obj, Lang, Out);
            }
            return false;
        }
    }

    // 2) 루트가 객체
    {
        TSharedPtr<FJsonObject> Root;
        const TSharedRef<TJsonReader<>> R = TJsonReaderFactory<>::Create(Json);
        if (!FJsonSerializer::Deserialize(R, Root) || !Root.IsValid()) return false;

        // { "itemsById": { "ART_001": {...} } }
        if (Root->HasTypedField<EJson::Object>(TEXT("itemsById")))
        {
            const TSharedPtr<FJsonObject> Map = Root->GetObjectField(TEXT("itemsById"));
            if (Map->HasTypedField<EJson::Object>(*ArtworkId))
            {
                return ParseDocentFromJsonObject(Map->GetObjectField(*ArtworkId), Lang, Out);
            }
        }

        // { "artworks":[...]} / { "items":[...]} / { "docs":[...] }
        for (const FString& Key : { TEXT("artworks"), TEXT("items"), TEXT("docs") })
        {
            const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
            if (Root->TryGetArrayField(Key, Arr))
            {
                for (const auto& V : *Arr)
                {
                    TSharedPtr<FJsonObject> Obj = V->AsObject();
                    if (!Obj.IsValid()) continue;
                    FString Id; Obj->TryGetStringField(TEXT("id"), Id);
                    if (Id == ArtworkId)
                        return ParseDocentFromJsonObject(Obj, Lang, Out);
                }
            }
        }

        // 루트 자체가 단일 작품일 수도 있음
        {
            FString Id; Root->TryGetStringField(TEXT("id"), Id);
            if (Id == ArtworkId)
                return ParseDocentFromJsonObject(Root, Lang, Out);
        }
    }
    return false;
}

static FString JoinPathLikeURL(const FString& Base, const FString& Relative)
{
    if (Base.EndsWith(TEXT("/"))) return Base + Relative;
    return Base + TEXT("/") + Relative;
}

static bool LoadLocalJsonString(const FString& FileOrFolder, const FString& ArtworkId, FString& OutJson)
{
    FString Path = FileOrFolder;
    if (FPaths::DirectoryExists(Path))
    {
        Path = FPaths::Combine(Path, ArtworkId + TEXT(".json"));
    }
    bool bOk = FFileHelper::LoadFileToString(OutJson, *Path);
    return bOk;
}

void UEX_BlueprintFunctionLibrary::FetchDocentForArtwork(const FString& BaseURL, const FString& ArtworkId, const FString& Lang, const FOnDocentFetched& Callback)
{
    auto Fail = [&Callback](int32 Code, const FString& Reason)
        {
            FDocentInfo Dummy;
            Callback.ExecuteIfBound(false, Code, Dummy);
            UE_LOG(LogTemp, Warning, TEXT("[Docent] Failed: %s (code=%d)"), *Reason, Code);
        };

    if (BaseURL.StartsWith(TEXT("file://")) || FPaths::DirectoryExists(BaseURL) || FPaths::FileExists(BaseURL))
    {
        FString Local = BaseURL.StartsWith(TEXT("file://")) ? BaseURL.RightChop(7) : BaseURL;

        FString Json;
        if (!FPaths::FileExists(Local))
        {
            if (!LoadLocalJsonString(Local, ArtworkId, Json))
            {
                Fail(-2, TEXT("Local JSON not found"));
                return;
            }
        }
        else
        {
            if (!FFileHelper::LoadFileToString(Json, *Local))
                return Fail(-3, TEXT("Cannot read local JSON"));
        }

        FDocentInfo Info;
        if (FindInCatalog(Json, ArtworkId, Lang, Info))
        {
            Callback.ExecuteIfBound(true, 200, Info);
            return;
        }
    }

    FString FinalURL = BaseURL;
    if (!BaseURL.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
    {
        // 폴더라고 보고 {Base}/{Id}.json 요청
        FinalURL = JoinPathLikeURL(BaseURL, ArtworkId + TEXT(".json"));
    }

    FHttpModule* Http = &FHttpModule::Get();
    if (!Http) return Fail(-1, TEXT("HttpModule not available"));

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http->CreateRequest();
    Req->SetVerb(TEXT("GET"));
    Req->SetURL(FinalURL);

    Req->OnProcessRequestComplete().BindLambda([Callback, BaseURL, ArtworkId, Lang](FHttpRequestPtr R, FHttpResponsePtr Resp, bool bOK)
        {
            if (!bOK || !Resp.IsValid())
            {
                FDocentInfo Dummy;
                Callback.ExecuteIfBound(false, -1, Dummy);
                return;
            }

            const int32 Code = Resp->GetResponseCode();
            const FString Body = Resp->GetContentAsString();

            FDocentInfo Info;
            if (FindInCatalog(Body, ArtworkId, Lang, Info))
            {
                Callback.ExecuteIfBound(true, Code, Info);
                return;
            }

            FDocentInfo Dummy;
            Callback.ExecuteIfBound(false, Code == 200 ? 404 : Code, Dummy);
        });

    Req->ProcessRequest();
}
