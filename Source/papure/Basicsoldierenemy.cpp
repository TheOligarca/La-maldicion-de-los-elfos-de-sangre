// BasicSoldierEnemy.cpp

#include "BasicSoldierEnemy.h"

ABasicSoldierEnemy::ABasicSoldierEnemy()
{
	EnemyType = EEnemyType::Basic;
	InitializeStats();
}

void ABasicSoldierEnemy::InitializeStats()
{
	// --- Soldado Raso ---
	// Vida: 1500
	// Armadura: Normal (5 puntos de reduccion)
	// Velocidad: Normal (300 cm/s)
	// Recompensa: 25 de oro
	// Dano a la base: 1

	Stats.MaxHealth = 1500.f;
	Stats.CurrentHealth = 1500.f;
	Stats.Armor = 5.f;
	Stats.MoveSpeed = 300.f;
	Stats.GoldReward = 25;
	Stats.DamageToBase = 1;
}

void ABasicSoldierEnemy::HandleDeath_Implementation()
{
	// Broadcast death event with gold reward
	// The WaveSpawner / GameManager will listen and add gold to the player
	OnDeath.Broadcast(this, Stats.GoldReward);

	// Optional: spawn gold pickup VFX at death location
	// You can override this in Blueprint to add particle effects, sounds, etc.
	// Example in BP: Spawn Emitter at Location -> Gold Coin particles

	Destroy();
}