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

	// �����ڿ����� MID ������ ����, �⺻ �ؽ�Ʈ ������ ����
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

	// ���̾ƿ�/��Ÿ�� ����
	TextComp->SetRelativeLocation(PanelOffset);

	// bFaceCamera�� ���� ���� ���� ���� ��ġ. �ո��� ���̵��� Yaw 180 ����.
	FRotator FixedRot = PanelRotation;
	FixedRot.Yaw += 180.f; // ���� ��ġ �� �¿���� ����
	TextComp->SetRelativeRotation(FixedRot);

	TextComp->SetWorldSize(TextWorldSize);
	TextComp->SetText(Description.IsEmpty() ? FText::FromString(TEXT("Enter description")) : Description);
	TextComp->SetVisibility(!bHiddenUntilOverlap);

	// CDO���� MID ���� ����
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	// �� �ν��Ͻ� ���� MID ����
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

			// �ؽ�Ʈ �ո��� ī�޶� ���� �ϴ� ȸ��:
			// ī�޶� �� �ؽ�Ʈ ������ '�ݴ�' ����(= �ؽ�Ʈ���� ī�޶� �ݴ� ����)�� �ٶ󺸰� �ϸ�
			// TextRender�� �ո��� ī�޶� ����.
			const FVector FromCam = TextComp->GetComponentLocation() - CamLoc;
			FRotator LookAt = FromCam.Rotation();

			// ����ڰ� �� �� ���� ����
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
