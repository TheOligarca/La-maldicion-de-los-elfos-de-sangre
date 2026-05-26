// EnemyFactory.cpp

#include "EnemyFactory.h"
#include "Engine/World.h"

UEnemyFactory::UEnemyFactory()
{
}

// -----------------------------------------------------------------
// Registration
// -----------------------------------------------------------------

void UEnemyFactory::RegisterArchetype(const FEnemyArchetype& Archetype)
{
	ArchetypeRegistry.Add(Archetype.Type, Archetype);
	UE_LOG(LogTemp, Log, TEXT("EnemyFactory: Registered archetype for type %d"), static_cast<int32>(Archetype.Type));
}

void UEnemyFactory::RegisterArchetypes(const TArray<FEnemyArchetype>& Archetypes)
{
	for (const FEnemyArchetype& Archetype : Archetypes)
	{
		RegisterArchetype(Archetype);
	}
}

// -----------------------------------------------------------------
// Spawning
// -----------------------------------------------------------------

AEnemyBase* UEnemyFactory::SpawnEnemy(
	UObject* WorldContext,
	EEnemyType Type,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	const TArray<FVector>& WaypointPath,
	int32 WaveNumber)
{
	if (!WorldContext)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyFactory::SpawnEnemy - WorldContext is null."));
		return nullptr;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyFactory::SpawnEnemy - Could not get World."));
		return nullptr;
	}

	// Look up archetype
	const FEnemyArchetype* Found = ArchetypeRegistry.Find(Type);
	if (!Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyFactory::SpawnEnemy - No archetype registered for type %d."), static_cast<int32>(Type));
		return nullptr;
	}

	if (!Found->EnemyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyFactory::SpawnEnemy - Archetype class is null for type %d."), static_cast<int32>(Type));
		return nullptr;
	}

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the enemy
	AEnemyBase* NewEnemy = World->SpawnActor<AEnemyBase>(Found->EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!NewEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyFactory::SpawnEnemy - SpawnActor failed."));
		return nullptr;
	}

	// Configure the enemy (accessible via friend class)
	NewEnemy->Stats = ScaleStatsForWave(Found->BaseStats, *Found, WaveNumber);
	NewEnemy->Stats.CurrentHealth = NewEnemy->Stats.MaxHealth;

	// Optional mesh override
	if (Found->MeshOverride)
	{
		if (USkeletalMeshComponent* MeshComp = NewEnemy->GetMesh())
		{
			MeshComp->SetSkeletalMesh(Found->MeshOverride);
		}
	}

	// Assign path
	NewEnemy->SetWaypointPath(WaypointPath);

	UE_LOG(LogTemp, Log, TEXT("EnemyFactory: Spawned enemy type %d at %s (Wave %d)"),
		static_cast<int32>(Type),
		*SpawnLocation.ToString(),
		WaveNumber);

	return NewEnemy;
}

TArray<AEnemyBase*> UEnemyFactory::SpawnEnemyBatch(
	UObject* WorldContext,
	EEnemyType Type,
	int32 Count,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	const TArray<FVector>& WaypointPath,
	int32 WaveNumber)
{
	TArray<AEnemyBase*> SpawnedEnemies;
	SpawnedEnemies.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
		AEnemyBase* Enemy = SpawnEnemy(WorldContext, Type, SpawnLocation, SpawnRotation, WaypointPath, WaveNumber);
		if (Enemy)
		{
			SpawnedEnemies.Add(Enemy);
		}
	}

	return SpawnedEnemies;
}

// -----------------------------------------------------------------
// Queries
// -----------------------------------------------------------------

bool UEnemyFactory::HasArchetype(EEnemyType Type) const
{
	return ArchetypeRegistry.Contains(Type);
}

FEnemyArchetype UEnemyFactory::GetArchetype(EEnemyType Type) const
{
	const FEnemyArchetype* Found = ArchetypeRegistry.Find(Type);
	return Found ? *Found : FEnemyArchetype();
}

// -----------------------------------------------------------------
// Internal
// -----------------------------------------------------------------

FEnemyStats UEnemyFactory::ScaleStatsForWave(const FEnemyStats& BaseStats, const FEnemyArchetype& Archetype, int32 Wave) const
{
	FEnemyStats Scaled = BaseStats;

	if (Wave > 1)
	{
		const float WaveExponent = static_cast<float>(Wave - 1);
		Scaled.MaxHealth = BaseStats.MaxHealth * FMath::Pow(Archetype.HealthScalePerWave, WaveExponent);
		Scaled.MoveSpeed = BaseStats.MoveSpeed * FMath::Pow(Archetype.SpeedScalePerWave, WaveExponent);
		Scaled.GoldReward = FMath::CeilToInt(BaseStats.GoldReward * (1.f + 0.05f * WaveExponent));
	}

	Scaled.CurrentHealth = Scaled.MaxHealth;
	return Scaled;
}
