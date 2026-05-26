// CrossbowTower.h
// Torre de Ballesta - Fires arrow projectiles at enemies.
// Type: Piercing | Damage: 50-100 | Speed: Fast | Cost: 100 gold

#pragma once

#include "CoreMinimal.h"
#include "TowerBase.h"
#include "ArrowProjectile.h"
#include "CrossbowTower.generated.h"

UCLASS(Blueprintable)
class PAPURE_API ACrossbowTower : public ATowerBase
{
	GENERATED_BODY()

public:
	ACrossbowTower();

protected:
	/** Spawns an arrow projectile aimed at the target. */
	virtual void FireAtTarget_Implementation(AEnemyBase* Target) override;

	/** Blueprint class for the arrow projectile. Assign in the editor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Crossbow")
	TSubclassOf<AArrowProjectile> ArrowClass;
};