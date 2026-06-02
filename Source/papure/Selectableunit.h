// SelectableUnit.h
// ActorComponent that marks any actor as selectable via the RTS selection box.
// Add this component to any unit the player should be able to select.
// Provides visual feedback (decal/circle) when selected.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SelectableUnit.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class PAPURE_API USelectableUnit : public UActorComponent
{
	GENERATED_BODY()

public:
	USelectableUnit();

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SetSelected(bool bNewSelected);

	UFUNCTION(BlueprintPure, Category = "Selection")
	bool IsSelected() const { return bIsSelected; }

	/** Visual indicator when selected (a decal or circle under the unit). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	UMaterialInterface* SelectionDecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FVector SelectionDecalSize = FVector(64.f, 64.f, 64.f);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UDecalComponent* SelectionDecal = nullptr;

	bool bIsSelected = false;

	void CreateSelectionDecal();
};