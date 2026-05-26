// TowerConstructor.cpp

#include "TowerConstructor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ATowerConstructor::ATowerConstructor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATowerConstructor::BeginPlay()
{
	Super::BeginPlay();

	CurrentGold = StartingGold;
	OnGoldChanged.Broadcast(CurrentGold);
}

void ATowerConstructor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInBuildMode && GhostActor)
	{
		UpdateGhostPosition();
		UpdateGhostMaterial();
	}
}

// -----------------------------------------------------------------
// Build Mode
// -----------------------------------------------------------------

void ATowerConstructor::StartBuildMode(TSubclassOf<ATowerBase> TowerClass)
{
	if (!TowerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TowerConstructor: TowerClass is null."));
		return;
	}

	// Cancel any previous build mode
	if (bIsInBuildMode)
	{
		CancelBuildMode();
	}

	PendingTowerClass = TowerClass;

	// Get the cost from the tower CDO (Class Default Object)
	if (const ATowerBase* CDO = TowerClass->GetDefaultObject<ATowerBase>())
	{
		PendingCost = CDO->GetTowerStats().Cost;
	}

	CreateGhost();
	bIsInBuildMode = true;

	UE_LOG(LogTemp, Log, TEXT("TowerConstructor: Entered build mode for %s (Cost: %d)"),
		*TowerClass->GetName(), PendingCost);
}

void ATowerConstructor::CancelBuildMode()
{
	DestroyGhost();
	PendingTowerClass = nullptr;
	PendingCost = 0;
	bIsInBuildMode = false;
	bIsValidPlacement = false;

	UE_LOG(LogTemp, Log, TEXT("TowerConstructor: Build mode cancelled."));
}

bool ATowerConstructor::PlaceTower()
{
	if (!bIsInBuildMode || !PendingTowerClass)
	{
		return false;
	}

	if (!bIsValidPlacement)
	{
		UE_LOG(LogTemp, Warning, TEXT("TowerConstructor: Invalid placement position."));
		return false;
	}

	if (CurrentGold < PendingCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("TowerConstructor: Not enough gold. Need %d, have %d."),
			PendingCost, CurrentGold);
		return false;
	}

	// Spawn the real tower
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ATowerBase* NewTower = World->SpawnActor<ATowerBase>(PendingTowerClass, GhostLocation, FRotator::ZeroRotator, SpawnParams);
	if (!NewTower)
	{
		UE_LOG(LogTemp, Error, TEXT("TowerConstructor: Failed to spawn tower."));
		return false;
	}

	// Deduct gold
	SpendGold(PendingCost);

	// Track placed towers
	PlacedTowers.Add(NewTower);

	// Notify
	OnTowerBuilt.Broadcast(NewTower, CurrentGold);

	UE_LOG(LogTemp, Log, TEXT("TowerConstructor: Placed %s at %s. Gold remaining: %d"),
		*PendingTowerClass->GetName(),
		*GhostLocation.ToString(),
		CurrentGold);

	// Exit build mode after placing
	CancelBuildMode();

	return true;
}

// -----------------------------------------------------------------
// Economy
// -----------------------------------------------------------------

void ATowerConstructor::AddGold(int32 Amount)
{
	CurrentGold += Amount;
	OnGoldChanged.Broadcast(CurrentGold);
}

bool ATowerConstructor::SpendGold(int32 Amount)
{
	if (CurrentGold < Amount)
	{
		return false;
	}

	CurrentGold -= Amount;
	OnGoldChanged.Broadcast(CurrentGold);
	return true;
}

void ATowerConstructor::SetCurrentGold(int32 Amount)
{
	CurrentGold = FMath::Max(Amount, 0);
	OnGoldChanged.Broadcast(CurrentGold);
}

// -----------------------------------------------------------------
// Ghost Preview
// -----------------------------------------------------------------

void ATowerConstructor::CreateGhost()
{
	UWorld* World = GetWorld();
	if (!World || !PendingTowerClass)
	{
		return;
	}

	// Spawn a temporary actor as the ghost preview
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GhostActor = World->SpawnActor<ATowerBase>(PendingTowerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (GhostActor)
	{
		// Disable collision and tick on the ghost so it doesn't target enemies
		GhostActor->SetActorEnableCollision(false);
		GhostActor->SetActorTickEnabled(false);

		// Make it slightly transparent if possible
		TArray<UStaticMeshComponent*> MeshComponents;
		GhostActor->GetComponents<UStaticMeshComponent>(MeshComponents);
		for (UStaticMeshComponent* Mesh : MeshComponents)
		{
			if (ValidPlacementMaterial)
			{
				for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
				{
					Mesh->SetMaterial(i, ValidPlacementMaterial);
				}
			}
		}
	}
}

void ATowerConstructor::DestroyGhost()
{
	if (GhostActor)
	{
		GhostActor->Destroy();
		GhostActor = nullptr;
	}
}

void ATowerConstructor::UpdateGhostPosition()
{
	const FVector MouseWorldPos = GetMouseWorldLocation();
	GhostLocation = SnapToGrid(MouseWorldPos);
	bIsValidPlacement = CheckPlacementValidity(GhostLocation);

	if (GhostActor)
	{
		GhostActor->SetActorLocation(GhostLocation);
	}
}

void ATowerConstructor::UpdateGhostMaterial()
{
	if (!GhostActor)
	{
		return;
	}

	UMaterialInterface* MaterialToUse = bIsValidPlacement ? ValidPlacementMaterial : InvalidPlacementMaterial;
	if (!MaterialToUse)
	{
		return;
	}

	TArray<UStaticMeshComponent*> MeshComponents;
	GhostActor->GetComponents<UStaticMeshComponent>(MeshComponents);
	for (UStaticMeshComponent* Mesh : MeshComponents)
	{
		for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
		{
			Mesh->SetMaterial(i, MaterialToUse);
		}
	}
}

// -----------------------------------------------------------------
// Placement Validation
// -----------------------------------------------------------------

bool ATowerConstructor::CheckPlacementValidity(const FVector& Location) const
{
	// Check 1: Is position on valid ground? (trace hit something)
	if (Location.IsNearlyZero())
	{
		return false;
	}

	// Check 2: Not overlapping with existing towers
	for (const ATowerBase* ExistingTower : PlacedTowers)
	{
		if (IsValid(ExistingTower))
		{
			const float Dist = FVector::Dist2D(Location, ExistingTower->GetActorLocation());
			if (Dist < MinTowerSpacing)
			{
				return false;
			}
		}
	}

	// Check 3: Enough gold
	if (CurrentGold < PendingCost)
	{
		return false;
	}

	return true;
}

FVector ATowerConstructor::SnapToGrid(const FVector& Location) const
{
	if (GridSize <= 0.f)
	{
		return Location;
	}

	FVector Snapped;
	Snapped.X = FMath::RoundToFloat(Location.X / GridSize) * GridSize;
	Snapped.Y = FMath::RoundToFloat(Location.Y / GridSize) * GridSize;
	Snapped.Z = Location.Z;
	return Snapped;
}

FVector ATowerConstructor::GetMouseWorldLocation() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return FVector::ZeroVector;
	}

	FVector WorldLocation, WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		return FVector::ZeroVector;
	}

	// Trace from the camera into the world to find where the mouse hits the ground
	FHitResult HitResult;
	const FVector TraceStart = WorldLocation;
	const FVector TraceEnd = TraceStart + WorldDirection * 10000.f;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	// Ignore the ghost and the constructor itself
	if (GhostActor)
	{
		QueryParams.AddIgnoredActor(GhostActor);
	}
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, PlacementTraceChannel, QueryParams))
	{
		return HitResult.Location;
	}

	return FVector::ZeroVector;
}