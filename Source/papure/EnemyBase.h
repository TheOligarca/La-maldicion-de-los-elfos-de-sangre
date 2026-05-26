// EnemyBase.h
// Base class for all enemies in the Tower Defense game.
// Enemies follow a waypoint path from spawn to goal.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

// Forward declaration needed because delegates reference AEnemyBase before it's defined
class AEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyReachedGoal, AEnemyBase*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyDeath, AEnemyBase*, Enemy, int32, RewardGold);

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Basic       UMETA(DisplayName = "Basic"),
	Fast        UMETA(DisplayName = "Fast"),
	Tank        UMETA(DisplayName = "Tank"),
	Flying      UMETA(DisplayName = "Flying"),
	Boss        UMETA(DisplayName = "Boss")
};

USTRUCT(BlueprintType)
struct FEnemyStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Armor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 GoldReward = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 DamageToBase = 1;
};

UCLASS(Abstract, Blueprintable)
class PAPURE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

	// Allow the factory to set Stats and mesh directly after spawning
	friend class UEnemyFactory;

public:
	AEnemyBase();

	// --- Waypoint Path System ---

	/** Assigns the ordered array of waypoints this enemy must follow. */
	UFUNCTION(BlueprintCallable, Category = "Path")
	void SetWaypointPath(const TArray<FVector>& InWaypoints);

	/** Returns the current target waypoint index. */
	UFUNCTION(BlueprintPure, Category = "Path")
	int32 GetCurrentWaypointIndex() const { return CurrentWaypointIndex; }

	/** Returns progress along the full path (0.0 = start, 1.0 = goal). */
	UFUNCTION(BlueprintPure, Category = "Path")
	float GetPathProgress() const;

	// --- Combat ---

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplySlow(float SlowMultiplier, float Duration);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const { return Stats.CurrentHealth > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	FEnemyStats GetStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	EEnemyType GetEnemyType() const { return EnemyType; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyReachedGoal OnReachedGoal;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDeath OnDeath;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Called when the enemy reaches the final waypoint. Override for custom behavior. */
	UFUNCTION(BlueprintNativeEvent, Category = "Path")
	void HandleReachedGoal();
	virtual void HandleReachedGoal_Implementation();

	/** Called when health reaches zero. Override for death FX, ragdoll, etc. */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void HandleDeath();
	virtual void HandleDeath_Implementation();

	/** Initializes default stats. Called from constructor or factory. */
	virtual void InitializeStats();

	// --- Properties ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FEnemyStats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	EEnemyType EnemyType = EEnemyType::Basic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	TArray<FVector> Waypoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	int32 CurrentWaypointIndex = 0;

	/** How close the enemy needs to be to a waypoint to consider it reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	float WaypointAcceptanceRadius = 50.f;

	/** Total accumulated distance of the full path (cached). */
	float TotalPathLength = 0.f;

	/** Distance covered so far. */
	float DistanceCovered = 0.f;

private:
	void MoveTowardsCurrentWaypoint(float DeltaTime);
	void AdvanceToNextWaypoint();
	void CachePathLength();

	// Slow effect
	float CurrentSpeedMultiplier = 1.f;
	float SlowTimer = 0.f;
};
