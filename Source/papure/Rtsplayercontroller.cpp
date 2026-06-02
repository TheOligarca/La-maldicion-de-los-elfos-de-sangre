// RTSPlayerController.cpp

#include "RTSPlayerController.h"
#include "RTSHUD.h"
#include "TowerConstructor.h"
#include "TowerBase.h"
#include "SelectableUnit.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

ARTSPlayerController::ARTSPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	RTSHUD = nullptr;
	MouseStartPosition = FVector2D::ZeroVector;
}

void ARTSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Cache the HUD reference
	RTSHUD = Cast<ARTSHUD>(GetHUD());

	// Auto-find TowerConstructor if not assigned
	if (!TowerConstructor)
	{
		TowerConstructor = Cast<ATowerConstructor>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ATowerConstructor::StaticClass()));
	}

	// Bind input in BeginPlay (works in UE 4.27 PlayerController)
	if (InputComponent)
	{
		InputComponent->BindAction("LeftClick", IE_Pressed, this, &ARTSPlayerController::OnLeftMousePressed);
		InputComponent->BindAction("LeftClick", IE_Released, this, &ARTSPlayerController::OnLeftMouseReleased);
		InputComponent->BindAction("RightClick", IE_Pressed, this, &ARTSPlayerController::OnRightMousePressed);
	}
}

void ARTSPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDragging && RTSHUD)
	{
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY))
		{
			RTSHUD->UpdateSelectionBox(FVector2D(MouseX, MouseY));
		}
	}
}

// -----------------------------------------------------------------
// Input Handlers
// -----------------------------------------------------------------

void ARTSPlayerController::OnLeftMousePressed()
{
	// If in build mode, place the tower
	if (bIsInBuildMode && TowerConstructor)
	{
		if (TowerConstructor->PlaceTower())
		{
			bIsInBuildMode = false;
		}
		return;
	}

	// Start drag selection
	float MouseX, MouseY;
	if (GetMousePosition(MouseX, MouseY))
	{
		MouseStartPosition = FVector2D(MouseX, MouseY);
		bIsDragging = true;

		if (RTSHUD)
		{
			RTSHUD->StartSelectionBox(MouseStartPosition);
		}
	}
}

void ARTSPlayerController::OnLeftMouseReleased()
{
	if (!bIsDragging)
	{
		return;
	}

	bIsDragging = false;

	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		if (RTSHUD)
		{
			RTSHUD->EndSelectionBox();
		}
		return;
	}

	const FVector2D MouseEndPosition(MouseX, MouseY);
	const float DragDistance = FVector2D::Distance(MouseStartPosition, MouseEndPosition);

	if (RTSHUD)
	{
		const FBox2D SelectionRect = RTSHUD->EndSelectionBox();

		if (DragDistance >= MinDragDistance)
		{
			PerformBoxSelection(SelectionRect);
		}
		else
		{
			PerformSingleSelection(MouseEndPosition);
		}
	}
	else
	{
		PerformSingleSelection(MouseEndPosition);
	}
}

void ARTSPlayerController::OnRightMousePressed()
{
	if (bIsInBuildMode)
	{
		ExitBuildMode();
		return;
	}

	if (SelectedUnits.Num() > 0)
	{
		FHitResult HitResult;
		if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			IssueMoveCommand(HitResult.Location);
		}
	}
}

// -----------------------------------------------------------------
// Selection Logic
// -----------------------------------------------------------------

void ARTSPlayerController::PerformSingleSelection(const FVector2D& ScreenPos)
{
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Pawn, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			USelectableUnit* Selectable = HitActor->FindComponentByClass<USelectableUnit>();
			if (Selectable)
			{
				const bool bShiftHeld = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
				SelectUnit(HitActor, bShiftHeld);
				return;
			}
		}
	}

	ClearSelection();
}

void ARTSPlayerController::PerformBoxSelection(const FBox2D& SelectionRect)
{
	const bool bShiftHeld = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);

	if (!bShiftHeld)
	{
		ClearSelection();
	}

	TArray<AActor*> ActorsInRect = GetSelectableActorsInRect(SelectionRect);

	for (AActor* Actor : ActorsInRect)
	{
		SelectUnit(Actor, true);
	}
}

TArray<AActor*> ARTSPlayerController::GetSelectableActorsInRect(const FBox2D& Rect) const
{
	TArray<AActor*> Result;

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		USelectableUnit* Selectable = Actor->FindComponentByClass<USelectableUnit>();
		if (!Selectable)
		{
			continue;
		}

		if (IsActorInScreenRect(Actor, Rect))
		{
			Result.Add(Actor);
		}
	}

	return Result;
}

bool ARTSPlayerController::IsActorInScreenRect(AActor* Actor, const FBox2D& Rect) const
{
	if (!Actor)
	{
		return false;
	}

	FVector2D ScreenPos;
	if (ProjectWorldLocationToScreen(Actor->GetActorLocation(), ScreenPos))
	{
		return Rect.IsInside(ScreenPos);
	}

	return false;
}

void ARTSPlayerController::SelectUnit(AActor* Unit, bool bAddToSelection)
{
	if (!Unit)
	{
		return;
	}

	USelectableUnit* Selectable = Unit->FindComponentByClass<USelectableUnit>();
	if (!Selectable)
	{
		return;
	}

	if (!bAddToSelection)
	{
		ClearSelection();
	}

	if (!SelectedUnits.Contains(Unit))
	{
		SelectedUnits.Add(Unit);
		Selectable->SetSelected(true);
	}
}

void ARTSPlayerController::ClearSelection()
{
	for (AActor* Unit : SelectedUnits)
	{
		if (IsValid(Unit))
		{
			USelectableUnit* Selectable = Unit->FindComponentByClass<USelectableUnit>();
			if (Selectable)
			{
				Selectable->SetSelected(false);
			}
		}
	}

	SelectedUnits.Empty();
}

// -----------------------------------------------------------------
// Build Mode
// -----------------------------------------------------------------

void ARTSPlayerController::EnterBuildMode(TSubclassOf<ATowerBase> TowerClass)
{
	if (!TowerConstructor)
	{
		UE_LOG(LogTemp, Warning, TEXT("RTSPlayerController: No TowerConstructor found."));
		return;
	}

	ClearSelection();
	TowerConstructor->StartBuildMode(TowerClass);
	bIsInBuildMode = true;
}

void ARTSPlayerController::ExitBuildMode()
{
	if (TowerConstructor)
	{
		TowerConstructor->CancelBuildMode();
	}

	bIsInBuildMode = false;
}

// -----------------------------------------------------------------
// Move Command
// -----------------------------------------------------------------

void ARTSPlayerController::IssueMoveCommand(const FVector& Destination)
{
	for (AActor* Unit : SelectedUnits)
	{
		if (!IsValid(Unit))
		{
			continue;
		}

		APawn* UnitPawn = Cast<APawn>(Unit);
		if (UnitPawn && UnitPawn->GetController())
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(UnitPawn->GetController(), Destination);
		}
	}
}