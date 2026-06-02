// SelectableUnit.cpp
// THIS IS SelectableUnit - NOT BasicSoldierEnemy

#include "SelectableUnit.h"
#include "GameFramework/Actor.h"

USelectableUnit::USelectableUnit()
{
	PrimaryComponentTick.bCanEverTick = false;
	SelectionDecal = nullptr;
}

void USelectableUnit::BeginPlay()
{
	Super::BeginPlay();
	CreateSelectionDecal();
}

void USelectableUnit::SetSelected(bool bNewSelected)
{
	bIsSelected = bNewSelected;

	if (SelectionDecal)
	{
		SelectionDecal->SetVisibility(bIsSelected);
	}
}

void USelectableUnit::CreateSelectionDecal()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	SelectionDecal = NewObject<UDecalComponent>(Owner, TEXT("SelectionDecal"));
	if (SelectionDecal)
	{
		SelectionDecal->SetupAttachment(Owner->GetRootComponent());
		SelectionDecal->RegisterComponent();
		SelectionDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
		SelectionDecal->DecalSize = SelectionDecalSize;

		if (SelectionDecalMaterial)
		{
			SelectionDecal->SetDecalMaterial(SelectionDecalMaterial);
		}

		SelectionDecal->SetVisibility(false);
	}
}