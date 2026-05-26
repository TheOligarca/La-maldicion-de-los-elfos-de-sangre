// CrossbowTower.cpp

#include "CrossbowTower.h"
#include "Engine/World.h"

ACrossbowTower::ACrossbowTower()
{
	// --- Crossbow Tower Stats (Torre de Ballesta) ---
	// Tipo de ataque: Perforante
	// Daño: 50 - 100
	// Velocidad de ataque: Rapida (2.5 attacks/sec)
	// Coste: 100 de oro

	TowerType = ETowerType::Crossbow;

	Stats.MinDamage = 50.f;
	Stats.MaxDamage = 100.f;
	Stats.FireRate = 2.5f;       // Fast attack speed
	Stats.AttackRange = 600.f;
	Stats.Cost = 100;
	Stats.UpgradeCost = 75;
}

void ACrossbowTower::FireAtTarget_Implementation(AEnemyBase* Target)
{
	if (!Target || !IsValid(Target) || !Target->IsAlive())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Determine arrow class (use default AArrowProjectile if none assigned)
	TSubclassOf<AArrowProjectile> SpawnClass = ArrowClass;
	if (!SpawnClass)
	{
		SpawnClass = AArrowProjectile::StaticClass();
	}

	// Spawn position and rotation from the fire point
	const FVector SpawnLocation = FirePoint->GetComponentLocation();
	const FRotator SpawnRotation = FirePoint->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AArrowProjectile* Arrow = World->SpawnActor<AArrowProjectile>(SpawnClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Arrow)
	{
		const float DamageRoll = GetRandomDamage();
		Arrow->InitProjectile(Target, DamageRoll);

		UE_LOG(LogTemp, Verbose, TEXT("CrossbowTower: Fired arrow at %s for %.1f damage"),
			*Target->GetName(), DamageRoll);
	}
}