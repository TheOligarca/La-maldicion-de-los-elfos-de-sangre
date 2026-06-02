// BasicSoldierEnemy.h
// Soldado Raso - Basic foot soldier enemy.
// Vida: 1500 | Armadura: Normal (5) | Velocidad: Normal (300)
// Drops gold on death for the player to build more towers.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BasicSoldierEnemy.generated.h"

UCLASS(Blueprintable)
class PAPURE_API ABasicSoldierEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABasicSoldierEnemy();

protected:
	virtual void HandleDeath_Implementation() override;
	virtual void InitializeStats() override;
};