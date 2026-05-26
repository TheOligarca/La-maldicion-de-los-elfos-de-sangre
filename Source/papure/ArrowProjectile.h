// ArrowProjectile.h
// Projectile fired by the Crossbow Tower.
// Flies towards the target enemy and deals piercing damage on hit.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBase.h"
#include "ArrowProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS(Blueprintable)
class PAPURE_API AArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	AArrowProjectile();

	/** Initialize the projectile with a target and damage. Call right after spawning. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitProjectile(AEnemyBase* InTarget, float InDamage);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// --- Config ---

	/** Speed of the arrow in cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Speed = 2000.f;

	/** Max lifetime in seconds before auto-destroy (if target is missed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxLifetime = 5.f;

private:
	void MoveTowardsTarget(float DeltaTime);
	void DestroyProjectile();

	UPROPERTY()
	AEnemyBase* TargetEnemy = nullptr;

	float Damage = 0.f;
	bool bHasHit = false;
};