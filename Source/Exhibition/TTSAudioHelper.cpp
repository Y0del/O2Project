#include "TTSAudioHelper.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "HAL/PlatformFilemanager.h"

bool UTTSAudioHelper::SaveBase64ToFile(const FString& Base64String, const FString& FileName, FString& OutFullPath)
{
    if (Base64String.IsEmpty())
    {
        return false;
    }

    TArray<uint8> DecodedBytes;
    if (!FBase64::Decode(Base64String, DecodedBytes))
    {
        return false;
    }

    FString SaveDir = FPaths::ProjectSavedDir() / TEXT("TTS");
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*SaveDir))
    {
        PlatformFile.CreateDirectory(*SaveDir);
    }

    OutFullPath = SaveDir / FileName;
    return FFileHelper::SaveArrayToFile(DecodedBytes, *OutFullPath);
}

FString UTTSAudioHelper::MakeFileUrlFromPath(const FString& FullPath)
{
    return FString(TEXT("file:///")) + FullPath;
}