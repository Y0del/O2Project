#include "EX_TextBox.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AEX_TextBox::AEX_TextBox()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetBoxExtent(FVector(80.f, 80.f, 80.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	TextComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text"));
	TextComp->SetupAttachment(Root);
	TextComp->SetHorizontalAlignment(EHorizTextAligment::EHTA_Left);
	TextComp->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	TextComp->SetWorldSize(TextWorldSize);
	TextComp->SetVisibility(!bHiddenUntilOverlap);
	TextComp->SetText(FText::FromString(TEXT("")));

	// 생성자에서는 MID 만들지 말고, 기본 텍스트 재질만 세팅
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMat(
		TEXT("/Engine/EngineMaterials/DefaultTextMaterialTranslucent.DefaultTextMaterialTranslucent"));
	if (BaseMat.Succeeded())
	{
		BaseTextMaterial = BaseMat.Object;
		TextComp->SetTextMaterial(BaseTextMaterial);
	}
	else
	{
		BaseTextMaterial = nullptr;
	}

	DynMat = nullptr;
}

void AEX_TextBox::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AEX_TextBox::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AEX_TextBox::OnOverlapEnd);
}

void AEX_TextBox::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 레이아웃/스타일 적용
	TextComp->SetRelativeLocation(PanelOffset);

	// bFaceCamera가 꺼져 있을 때는 고정 배치. 앞면이 보이도록 Yaw 180 보정.
	FRotator FixedRot = PanelRotation;
	FixedRot.Yaw += 180.f; // 고정 배치 시 좌우반전 방지
	TextComp->SetRelativeRotation(FixedRot);

	TextComp->SetWorldSize(TextWorldSize);
	TextComp->SetText(Description.IsEmpty() ? FText::FromString(TEXT("Enter description")) : Description);
	TextComp->SetVisibility(!bHiddenUntilOverlap);

	// CDO에서 MID 생성 금지
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	// 이 인스턴스 전용 MID 생성
	if (!DynMat && BaseTextMaterial)
	{
		DynMat = UMaterialInstanceDynamic::Create(BaseTextMaterial, this);
		if (DynMat)
		{
			TextComp->SetTextMaterial(DynMat);
		}
	}

	if (DynMat)
	{
		FLinearColor Final = HoloColor;
		Final.A = HoloOpacity;
		DynMat->SetVectorParameterValue(TEXT("FontColor"), Final);
	}
}

void AEX_TextBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFaceCamera && TextComp->IsVisible())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			FVector CamLoc; FRotator CamRot;
			PC->GetPlayerViewPoint(CamLoc, CamRot);

			// 텍스트 앞면이 카메라를 보게 하는 회전:
			// 카메라 → 텍스트 벡터의 '반대' 방향(= 텍스트에서 카메라 반대 방향)을 바라보게 하면
			// TextRender의 앞면이 카메라를 향함.
			const FVector FromCam = TextComp->GetComponentLocation() - CamLoc;
			FRotator LookAt = FromCam.Rotation();

			// 사용자가 준 롤 기울기 유지
			LookAt.Roll = TextComp->GetRelativeRotation().Roll + PanelRotation.Roll;

			TextComp->SetWorldRotation(LookAt);
		}
	}
}

void AEX_TextBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (OtherActor && OtherActor == PlayerPawn)
	{
		TextComp->SetVisibility(true);
	}
}

void AEX_TextBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (OtherActor && OtherActor == PlayerPawn && bHiddenUntilOverlap)
	{
		TextComp->SetVisibility(false);
	}
}
