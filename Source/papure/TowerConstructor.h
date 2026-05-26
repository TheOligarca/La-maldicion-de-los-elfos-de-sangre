// TowerConstructor.h
// Handles tower placement in the level.
// Shows a ghost preview that follows the mouse cursor,
// validates placement, snaps to grid, and deducts gold on build.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerBase.h"
#include "TowerConstructor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerBuilt, ATowerBase*, Tower, int32, RemainingGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

UCLASS(Blueprintable)
class PAPURE_API ATowerConstructor : public AActor
{
	GENERATED_BODY()

public:
	ATowerConstructor();

	// -----------------------------------------------------------------
	// Build Mode API
	// -----------------------------------------------------------------

	/** Enter build mode with a specific tower class. Shows the ghost preview. */
	UFUNCTION(BlueprintCallable, Category = "Constructor")
	void StartBuildMode(TSubclassOf<ATowerBase> TowerClass);

	/** Cancel build mode and hide the ghost. */
	UFUNCTION(BlueprintCallable, Category = "Constructor")
	void CancelBuildMode();

	/** Attempt to place the tower at the current ghost position. */
	UFUNCTION(BlueprintCallable, Category = "Constructor")
	bool PlaceTower();

	/** Is the player currently in build mode? */
	UFUNCTION(BlueprintPure, Category = "Constructor")
	bool IsInBuildMode() const { return bIsInBuildMode; }

	/** Is the current ghost position a valid placement? */
	UFUNCTION(BlueprintPure, Category = "Constructor")
	bool IsValidPlacement() const { return bIsValidPlacement; }

	// -----------------------------------------------------------------
	// Gold / Economy
	// -----------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Economy")
	int32 GetCurrentGold() const { return CurrentGold; }

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void SetCurrentGold(int32 Amount);

	// -----------------------------------------------------------------
	// Events
	// -----------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTowerBuilt OnTowerBuilt;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGoldChanged OnGoldChanged;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// -----------------------------------------------------------------
	// Configuration
	// -----------------------------------------------------------------

	/** Starting gold for the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 StartingGold = 500;

	/** Grid cell size for snapping (0 = no snap). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	float GridSize = 100.f;

	/** Channel to trace for ground placement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	TEnumAsByte<ECollisionChannel> PlacementTraceChannel = ECC_Visibility;

	/** Minimum distance between towers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	float MinTowerSpacing = 120.f;

	/** Material for valid placement (green tint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	UMaterialInterface* ValidPlacementMaterial;

	/** Material for invalid placement (red tint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	UMaterialInterface* InvalidPlacementMaterial;

private:
	void UpdateGhostPosition();
	void UpdateGhostMaterial();
	bool CheckPlacementValidity(const FVector& Location) const;
	FVector SnapToGrid(const FVector& Location) const;
	FVector GetMouseWorldLocation() const;
	void CreateGhost();
	void DestroyGhost();

	UPROPERTY()
	TSubclassOf<ATowerBase> PendingTowerClass;

	UPROPERTY()
	AActor* GhostActor = nullptr;

	UPROPERTY()
	TArray<ATowerBase*> PlacedTowers;

	int32 CurrentGold = 0;
	int32 PendingCost = 0;
	bool bIsInBuildMode = false;
	bool bIsValidPlacement = false;
	FVector GhostLocation = FVector::ZeroVector;
};