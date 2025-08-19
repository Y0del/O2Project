#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EX_TextBox.generated.h"

class UBoxComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS()
class EXHIBITION_API AEX_TextBox : public AActor
{
	GENERATED_BODY()

public:
	AEX_TextBox();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 런타임 전용 동적 재질(저장 안 됨)
	UPROPERTY(Transient)
	UMaterialInstanceDynamic* DynMat;

	// MID를 만들 원본 재질
	UPROPERTY(EditDefaultsOnly, Category = "TextBox|Style")
	UMaterialInterface* BaseTextMaterial;

public:
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTextRenderComponent* TextComp;

	// Editable properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Content", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Layout")
	FVector PanelOffset = FVector(50.f, 80.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Layout")
	FRotator PanelRotation = FRotator(0.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Style", meta = (ClampMin = "5.0"))
	float TextWorldSize = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Style")
	FLinearColor HoloColor = FLinearColor(0.25f, 0.9f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Style", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HoloOpacity = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Behavior")
	bool bHiddenUntilOverlap = true;

	// 카메라 향하기(빌보드 효과)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextBox|Behavior")
	bool bFaceCamera = false;
};
