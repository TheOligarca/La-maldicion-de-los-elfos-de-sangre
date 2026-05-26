// WaveSpawner.cpp

#include "WaveSpawner.h"

AWaveSpawner::AWaveSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWaveSpawner::BeginPlay()
{
	Super::BeginPlay();

	// Create and configure the factory
	EnemyFactory = NewObject<UEnemyFactory>(this, UEnemyFactory::StaticClass());
	EnemyFactory->RegisterArchetypes(EnemyArchetypes);

	// Cache the waypoint path from placed actors
	CachedPath = BuildWaypointPath();

	if (CachedPath.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("WaveSpawner: Need at least 2 waypoint actors to define a path."));
	}
}

void AWaveSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPaused || bAllWavesDone)
	{
		return;
	}

	// Waiting between waves
	if (bWaitingForNextWave)
	{
		WaveDelayTimer -= DeltaTime;
		if (WaveDelayTimer <= 0.f)
		{
			bWaitingForNextWave = false;
			BeginWave(CurrentWaveIndex + 1);
		}
		return;
	}

	// Spawning enemies within a wave
	if (bIsSpawning)
	{
		SpawnTimer -= DeltaTime;
		if (SpawnTimer <= 0.f)
		{
			SpawnNextInGroup();
		}
	}
}

// -----------------------------------------------------------------
// Public API
// -----------------------------------------------------------------

void AWaveSpawner::StartSpawning()
{
	if (Waves.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveSpawner: No waves defined."));
		return;
	}

	bIsPaused = false;

	if (CurrentWaveIndex < 0)
	{
		BeginWave(0);
	}
}

void AWaveSpawner::ForceNextWave()
{
	bWaitingForNextWave = false;
	const int32 NextWave = CurrentWaveIndex + 1;
	if (NextWave < Waves.Num())
	{
		BeginWave(NextWave);
	}
}

void AWaveSpawner::SetPaused(bool bPause)
{
	bIsPaused = bPause;
}

// -----------------------------------------------------------------
// Internal: Wave Lifecycle
// -----------------------------------------------------------------

void AWaveSpawner::BeginWave(int32 WaveIndex)
{
	if (!Waves.IsValidIndex(WaveIndex))
	{
		bAllWavesDone = true;
		OnAllWavesCompleted.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("WaveSpawner: All waves completed!"));
		return;
	}

	CurrentWaveIndex = WaveIndex;
	CurrentGroupIndex = 0;
	SpawnedInGroup = 0;
	bIsSpawning = true;
	SpawnTimer = 0.f; // Spawn first enemy immediately

	OnWaveStarted.Broadcast(CurrentWaveIndex + 1);
	UE_LOG(LogTemp, Log, TEXT("WaveSpawner: Starting %s (Wave %d/%d)"),
		*Waves[WaveIndex].WaveName,
		WaveIndex + 1,
		Waves.Num());
}

void AWaveSpawner::SpawnNextInGroup()
{
	const FWaveDefinition& CurrentWave = Waves[CurrentWaveIndex];

	if (!CurrentWave.EnemyGroups.IsValidIndex(CurrentGroupIndex))
	{
		// All groups in this wave have been fully spawned
		bIsSpawning = false;
		CheckWaveComplete();
		return;
	}

	const FEnemyGroup& Group = CurrentWave.EnemyGroups[CurrentGroupIndex];

	// Spawn one enemy
	AEnemyBase* Enemy = EnemyFactory->SpawnEnemy(
		this,
		Group.EnemyType,
		CachedPath.Num() > 0 ? CachedPath[0] : GetActorLocation(),
		GetActorRotation(),
		CachedPath,
		CurrentWaveIndex + 1  // wave number (1-based)
	);

	if (Enemy)
	{
		AliveEnemies.Add(Enemy);
		Enemy->OnDeath.AddDynamic(this, &AWaveSpawner::OnEnemyDeath);
		Enemy->OnReachedGoal.AddDynamic(this, &AWaveSpawner::OnEnemyReachedGoal);
	}

	SpawnedInGroup++;

	if (SpawnedInGroup >= Group.Count)
	{
		// Move to next group
		CurrentGroupIndex++;
		SpawnedInGroup = 0;

		if (CurrentWave.EnemyGroups.IsValidIndex(CurrentGroupIndex))
		{
			SpawnTimer = CurrentWave.EnemyGroups[CurrentGroupIndex].SpawnInterval;
		}
		else
		{
			// Finished all groups
			bIsSpawning = false;
			CheckWaveComplete();
		}
	}
	else
	{
		SpawnTimer = Group.SpawnInterval;
	}
}

// -----------------------------------------------------------------
// Internal: Enemy Events
// -----------------------------------------------------------------

void AWaveSpawner::OnEnemyDeath(AEnemyBase* Enemy, int32 Gold)
{
	AliveEnemies.Remove(Enemy);
	// Gold reward can be sent to the player economy system here
	CheckWaveComplete();
}

void AWaveSpawner::OnEnemyReachedGoal(AEnemyBase* Enemy)
{
	AliveEnemies.Remove(Enemy);
	// Damage to player base can be applied here
	CheckWaveComplete();
}

void AWaveSpawner::CheckWaveComplete()
{
	// Wave is complete when spawning is done AND all enemies are dead or reached goal
	if (bIsSpawning || AliveEnemies.Num() > 0)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("WaveSpawner: Wave %d completed."), CurrentWaveIndex + 1);
	OnWaveCompleted.Broadcast(CurrentWaveIndex + 1);

	// Check if there are more waves
	if (CurrentWaveIndex + 1 < Waves.Num())
	{
		bWaitingForNextWave = true;
		WaveDelayTimer = Waves[CurrentWaveIndex].DelayAfterWave;
	}
	else
	{
		bAllWavesDone = true;
		OnAllWavesCompleted.Broadcast();
	}
}

// -----------------------------------------------------------------
// Path Builder
// -----------------------------------------------------------------

TArray<FVector> AWaveSpawner::BuildWaypointPath() const
{
	TArray<FVector> Path;
	Path.Reserve(WaypointActors.Num());

	for (const AActor* WP : WaypointActors)
	{
		if (WP)
		{
			Path.Add(WP->GetActorLocation());
		}
	}

	return Path;
}