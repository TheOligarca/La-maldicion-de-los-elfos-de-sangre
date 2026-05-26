// WaveSpawner.h
// Actor placed in the level that manages enemy waves.
// Reads waypoints from placed WaypointActor markers,
// then uses UEnemyFactory to spawn enemies at intervals.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBase.h"
#include "EnemyFactory.h"
#include "WaveSpawner.generated.h"

/**
 * Defines a group of enemies within a wave.
 * A single wave can have multiple groups (e.g. 5 Basic + 2 Fast).
 */
USTRUCT(BlueprintType)
struct FEnemyGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	EEnemyType EnemyType = EEnemyType::Basic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "1"))
	int32 Count = 5;

	/** Seconds between each spawn in this group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0.1"))
	float SpawnInterval = 1.0f;
};

/**
 * A full wave: one or more enemy groups spawned in sequence.
 */
USTRUCT(BlueprintType)
struct FWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FString WaveName = TEXT("Wave");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FEnemyGroup> EnemyGroups;

	/** Delay in seconds before the NEXT wave starts after this one finishes spawning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (ClampMin = "0.0"))
	float DelayAfterWave = 5.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);

UCLASS(Blueprintable)
class PAPURE_API AWaveSpawner : public AActor
{
	GENERATED_BODY()

public:
	AWaveSpawner();

	// --- Wave Control ---

	/** Start the first wave (or resume after pause). */
	UFUNCTION(BlueprintCallable, Category = "Waves")
	void StartSpawning();

	/** Force-start the next wave immediately. */
	UFUNCTION(BlueprintCallable, Category = "Waves")
	void ForceNextWave();

	/** Pause / resume spawning. */
	UFUNCTION(BlueprintCallable, Category = "Waves")
	void SetPaused(bool bPause);

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetTotalWaves() const { return Waves.Num(); }

	UFUNCTION(BlueprintPure, Category = "Waves")
	int32 GetAliveEnemyCount() const { return AliveEnemies.Num(); }

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaveCompleted OnWaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllWavesCompleted OnAllWavesCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Configuration ---

	/** The enemy factory (created automatically). */
	UPROPERTY()
	UEnemyFactory* EnemyFactory;

	/** Archetype table: register one entry per enemy type you use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory")
	TArray<FEnemyArchetype> EnemyArchetypes;

	/** Ordered waves to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TArray<FWaveDefinition> Waves;

	/**
	 * Waypoint actors placed in the level.
	 * The enemy path goes from index 0 (spawn) to the last index (goal).
	 * Place them in order in the Editor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	TArray<AActor*> WaypointActors;

private:
	void BeginWave(int32 WaveIndex);
	void SpawnNextInGroup();

	UFUNCTION()
	void OnEnemyDeath(AEnemyBase* Enemy, int32 Gold);

	UFUNCTION()
	void OnEnemyReachedGoal(AEnemyBase* Enemy);

	void CheckWaveComplete();
	TArray<FVector> BuildWaypointPath() const;

	// Runtime state
	TArray<FVector> CachedPath;
	TArray<AEnemyBase*> AliveEnemies;

	int32 CurrentWaveIndex = -1;
	int32 CurrentGroupIndex = 0;
	int32 SpawnedInGroup = 0;
	float SpawnTimer = 0.f;
	float WaveDelayTimer = 0.f;
	bool bIsSpawning = false;
	bool bIsPaused = false;
	bool bWaitingForNextWave = false;
	bool bAllWavesDone = false;
};
