// TowerBase.h
// Base class for all towers in the Tower Defense game.
// Handles enemy detection, targeting, and fire rate timing.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBase.h"
#include "Components/SphereComponent.h"
#include "TowerBase.generated.h"

UENUM(BlueprintType)
enum class ETowerType : uint8
{
	Crossbow    UMETA(DisplayName = "Crossbow"),
	Cannon      UMETA(DisplayName = "Cannon"),
	Magic       UMETA(DisplayName = "Magic"),
	Slow        UMETA(DisplayName = "Slow")
};

UENUM(BlueprintType)
enum class ETargetingMode : uint8
{
	First       UMETA(DisplayName = "First (most path progress)"),
	Last        UMETA(DisplayName = "Last (least path progress)"),
	Closest     UMETA(DisplayName = "Closest"),
	Strongest   UMETA(DisplayName = "Strongest (most HP)")
};

USTRUCT(BlueprintType)
struct FTowerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MinDamage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxDamage = 100.f;

	/** Attacks per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float FireRate = 2.0f;

	/** Detection and attack radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Cost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 UpgradeCost = 75;
};

UCLASS(Abstract, Blueprintable)
class PAPURE_API ATowerBase : public AActor
{
	GENERATED_BODY()

public:
	ATowerBase();

	UFUNCTION(BlueprintPure, Category = "Tower")
	FTowerStats GetTowerStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Tower")
	ETowerType GetTowerType() const { return TowerType; }

	UFUNCTION(BlueprintPure, Category = "Tower")
	AEnemyBase* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void SetTargetingMode(ETargetingMode NewMode) { TargetMode = NewMode; }

	/** Returns a random damage value between MinDamage and MaxDamage. */
	UFUNCTION(BlueprintPure, Category = "Tower")
	float GetRandomDamage() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Override in subclasses to spawn projectiles, play FX, etc. */
	UFUNCTION(BlueprintNativeEvent, Category = "Tower")
	void FireAtTarget(AEnemyBase* Target);
	virtual void FireAtTarget_Implementation(AEnemyBase* Target);

	/** Called when a new target is acquired. */
	UFUNCTION(BlueprintNativeEvent, Category = "Tower")
	void OnTargetAcquired(AEnemyBase* Target);
	virtual void OnTargetAcquired_Implementation(AEnemyBase* Target);

	/** Called when the current target is lost. */
	UFUNCTION(BlueprintNativeEvent, Category = "Tower")
	void OnTargetLost();
	virtual void OnTargetLost_Implementation();

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* RangeCollider;

	/** Where projectiles spawn from. Position this at the tip of the tower in Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* FirePoint;

	// --- Config ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FTowerStats Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	ETowerType TowerType = ETowerType::Crossbow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	ETargetingMode TargetMode = ETargetingMode::First;

private:
	UFUNCTION()
	void OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void UpdateTarget();
	void RotateTowardsTarget(float DeltaTime);
	AEnemyBase* SelectBestTarget() const;

	UPROPERTY()
	TArray<AEnemyBase*> EnemiesInRange;

	UPROPERTY()
	AEnemyBase* CurrentTarget = nullptr;

	float FireCooldown = 0.f;
};