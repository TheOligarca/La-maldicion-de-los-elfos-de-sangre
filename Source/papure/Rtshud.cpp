// RTSHUD.cpp

#include "RTSHUD.h"
#include "Engine/Canvas.h"

ARTSHUD::ARTSHUD()
{
}

void ARTSHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bIsDrawing)
	{
		return;
	}

	// Calculate the rectangle corners
	const float MinX = FMath::Min(SelectionStart.X, SelectionEnd.X);
	const float MinY = FMath::Min(SelectionStart.Y, SelectionEnd.Y);
	const float MaxX = FMath::Max(SelectionStart.X, SelectionEnd.X);
	const float MaxY = FMath::Max(SelectionStart.Y, SelectionEnd.Y);
	const float Width = MaxX - MinX;
	const float Height = MaxY - MinY;

	// Draw filled rectangle (semi-transparent)
	DrawRect(BoxFillColor, MinX, MinY, Width, Height);

	// Draw border lines
	const FLinearColor BorderCol = BoxBorderColor;
	const float T = BoxBorderThickness;

	// Top
	DrawLine(MinX, MinY, MaxX, MinY, BorderCol, T);
	// Bottom
	DrawLine(MinX, MaxY, MaxX, MaxY, BorderCol, T);
	// Left
	DrawLine(MinX, MinY, MinX, MaxY, BorderCol, T);
	// Right
	DrawLine(MaxX, MinY, MaxX, MaxY, BorderCol, T);
}

void ARTSHUD::StartSelectionBox(const FVector2D& StartPos)
{
	SelectionStart = StartPos;
	SelectionEnd = StartPos;
	bIsDrawing = true;
}

void ARTSHUD::UpdateSelectionBox(const FVector2D& CurrentPos)
{
	SelectionEnd = CurrentPos;
}

FBox2D ARTSHUD::EndSelectionBox()
{
	bIsDrawing = false;

	const FVector2D Min(
		FMath::Min(SelectionStart.X, SelectionEnd.X),
		FMath::Min(SelectionStart.Y, SelectionEnd.Y)
	);
	const FVector2D Max(
		FMath::Max(SelectionStart.X, SelectionEnd.X),
		FMath::Max(SelectionStart.Y, SelectionEnd.Y)
	);

	return FBox2D(Min, Max);
}

FBox2D ARTSHUD::GetSelectionRect() const
{
	const FVector2D Min(
		FMath::Min(SelectionStart.X, SelectionEnd.X),
		FMath::Min(SelectionStart.Y, SelectionEnd.Y)
	);
	const FVector2D Max(
		FMath::Max(SelectionStart.X, SelectionEnd.X),
		FMath::Max(SelectionStart.Y, SelectionEnd.Y)
	);

	return FBox2D(Min, Max);
}