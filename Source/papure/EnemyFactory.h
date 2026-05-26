// EnemyFactory.h
// Factory pattern implementation for spawning different enemy types.
// Centralizes enemy creation, stat configuration and path assignment.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EnemyBase.h"
#include "EnemyFactory.generated.h"

/**
 * Configuration data asset for defining enemy archetypes.
 * Create one Data Asset per enemy type in the Editor.
 */
USTRUCT(BlueprintType)
struct FEnemyArchetype
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
	EEnemyType Type = EEnemyType::Basic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
	FEnemyStats BaseStats;

	/** Mesh / skeletal mesh override (optional, set in Blueprint subclass). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
	USkeletalMesh* MeshOverride = nullptr;

	/** Stat multiplier applied per wave (1.0 = no scaling). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling", meta = (ClampMin = "1.0"))
	float HealthScalePerWave = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling", meta = (ClampMin = "1.0"))
	float SpeedScalePerWave = 1.02f;
};

/**
 * UEnemyFactory
 *
 * Singleton-style UObject that lives on the GameMode or a manager actor.
 * Responsible for:
 *   - Registering enemy archetypes
 *   - Spawning enemies by type
 *   - Applying per-wave stat scaling
 *   - Assigning waypoint paths to spawned enemies
 */
UCLASS(Blueprintable, BlueprintType)
class PAPURE_API UEnemyFactory : public UObject
{
	GENERATED_BODY()

public:
	UEnemyFactory();

	// -----------------------------------------------------------------
	// Registration
	// -----------------------------------------------------------------

	/** Register an archetype. Call during initialization. */
	UFUNCTION(BlueprintCallable, Category = "Factory")
	void RegisterArchetype(const FEnemyArchetype& Archetype);

	/** Bulk registration from a data table or array. */
	UFUNCTION(BlueprintCallable, Category = "Factory")
	void RegisterArchetypes(const TArray<FEnemyArchetype>& Archetypes);

	// -----------------------------------------------------------------
	// Spawning
	// -----------------------------------------------------------------

	/**
	 * Spawns an enemy of the given type at SpawnLocation,
	 * assigns it the waypoint path, and scales its stats by WaveNumber.
	 *
	 * @param WorldContext   Any world-context object (e.g. GameMode, an Actor).
	 * @param Type           Which enemy archetype to spawn.
	 * @param SpawnLocation  World location to spawn the enemy.
	 * @param SpawnRotation  Initial rotation.
	 * @param WaypointPath   Ordered positions from spawn to goal.
	 * @param WaveNumber     Current wave (1-based). Stats scale accordingly.
	 * @return               The spawned enemy, or nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Factory", meta = (WorldContext = "WorldContext"))
	AEnemyBase* SpawnEnemy(
		UObject* WorldContext,
		EEnemyType Type,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		const TArray<FVector>& WaypointPath,
		int32 WaveNumber = 1
	);

	/**
	 * Convenience: spawn a batch of the same type.
	 * Returns all spawned enemies.
	 */
	UFUNCTION(BlueprintCallable, Category = "Factory", meta = (WorldContext = "WorldContext"))
	TArray<AEnemyBase*> SpawnEnemyBatch(
		UObject* WorldContext,
		EEnemyType Type,
		int32 Count,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		const TArray<FVector>& WaypointPath,
		int32 WaveNumber = 1
	);

	// -----------------------------------------------------------------
	// Queries
	// -----------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Factory")
	bool HasArchetype(EEnemyType Type) const;

	UFUNCTION(BlueprintPure, Category = "Factory")
	FEnemyArchetype GetArchetype(EEnemyType Type) const;

private:
	/** Registry: maps each EEnemyType to its archetype data. */
	UPROPERTY()
	TMap<EEnemyType, FEnemyArchetype> ArchetypeRegistry;

	/** Apply wave scaling to base stats and return the modified copy. */
	FEnemyStats ScaleStatsForWave(const FEnemyStats& BaseStats, const FEnemyArchetype& Archetype, int32 Wave) const;
};
