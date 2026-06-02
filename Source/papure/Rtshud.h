// RTSHUD.h
// HUD class that draws the RTS selection rectangle (marquee box).
// Assign this as your HUD class in the GameMode.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RTSHUD.generated.h"

UCLASS()
class PAPURE_API ARTSHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARTSHUD();

	virtual void DrawHUD() override;

	/** Start drawing the selection box from this screen point. */
	void StartSelectionBox(const FVector2D& StartPos);

	/** Update the end point as the mouse moves. */
	void UpdateSelectionBox(const FVector2D& CurrentPos);

	/** Stop drawing and return the final rectangle. */
	FBox2D EndSelectionBox();

	/** Is the box currently being drawn? */
	bool IsDrawingSelectionBox() const { return bIsDrawing; }

	/** Get current selection rectangle in screen space. */
	FBox2D GetSelectionRect() const;

protected:
	/** Color of the selection box border. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FLinearColor BoxBorderColor = FLinearColor(0.f, 1.f, 0.f, 1.f);

	/** Color of the selection box fill. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FLinearColor BoxFillColor = FLinearColor(0.f, 1.f, 0.f, 0.15f);

	/** Border thickness. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	float BoxBorderThickness = 2.f;

private:
	FVector2D SelectionStart = FVector2D::ZeroVector;
	FVector2D SelectionEnd = FVector2D::ZeroVector;
	bool bIsDrawing = false;
};