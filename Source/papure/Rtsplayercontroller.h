// RTSPlayerController.h
// Player controller for the Tower Defense game with RTS-style unit selection.
// Left click drag = marquee selection | Left click = single select / place tower
// Right click = move command / cancel build | Shift = add to selection

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSPlayerController.generated.h"

class ATowerConstructor;
class ATowerBase;
class ARTSHUD;
class USelectableUnit;

UCLASS(Blueprintable)
class PAPURE_API ARTSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARTSPlayerController();

	// -----------------------------------------------------------------
	// Selection
	// -----------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Selection")
	TArray<AActor*> GetSelectedUnits() const { return SelectedUnits; }

	UFUNCTION(BlueprintPure, Category = "Selection")
	int32 GetSelectedCount() const { return SelectedUnits.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectUnit(AActor* Unit, bool bAddToSelection = false);

	// -----------------------------------------------------------------
	// Build Mode
	// -----------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Build")
	void EnterBuildMode(TSubclassOf<ATowerBase> TowerClass);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ExitBuildMode();

	/** Assign in the level or it will auto-find. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	ATowerConstructor* TowerConstructor;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	float MinDragDistance = 10.f;

private:
	// Input handlers
	void OnLeftMousePressed();
	void OnLeftMouseReleased();
	void OnRightMousePressed();

	// Selection logic
	void PerformSingleSelection(const FVector2D& ScreenPos);
	void PerformBoxSelection(const FBox2D& SelectionRect);
	TArray<AActor*> GetSelectableActorsInRect(const FBox2D& Rect) const;
	bool IsActorInScreenRect(AActor* Actor, const FBox2D& Rect) const;

	// Move command
	void IssueMoveCommand(const FVector& Destination);

	UPROPERTY()
	TArray<AActor*> SelectedUnits;

	UPROPERTY()
	ARTSHUD* RTSHUD;

	FVector2D MouseStartPosition;
	bool bIsDragging = false;
	bool bIsInBuildMode = false;
};