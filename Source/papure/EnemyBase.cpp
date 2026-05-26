// EnemyBase.cpp

#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Disable AI controller by default; movement is manual via waypoints
	AutoPossessAI = EAutoPossessAI::Disabled;

	InitializeStats();
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MovComp = GetCharacterMovement())
	{
		MovComp->MaxWalkSpeed = Stats.MoveSpeed;
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsAlive() || Waypoints.Num() == 0)
	{
		return;
	}

	// Handle slow timer
	if (SlowTimer > 0.f)
	{
		SlowTimer -= DeltaTime;
		if (SlowTimer <= 0.f)
		{
			CurrentSpeedMultiplier = 1.f;
			SlowTimer = 0.f;
		}
	}

	MoveTowardsCurrentWaypoint(DeltaTime);
}

// -----------------------------------------------------------------
// Path System
// -----------------------------------------------------------------

void AEnemyBase::SetWaypointPath(const TArray<FVector>& InWaypoints)
{
	Waypoints = InWaypoints;
	CurrentWaypointIndex = 0;
	DistanceCovered = 0.f;
	CachePathLength();
}

float AEnemyBase::GetPathProgress() const
{
	if (TotalPathLength <= 0.f)
	{
		return 0.f;
	}
	return FMath::Clamp(DistanceCovered / TotalPathLength, 0.f, 1.f);
}

void AEnemyBase::MoveTowardsCurrentWaypoint(float DeltaTime)
{
	if (!Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		return;
	}

	const FVector TargetLocation = Waypoints[CurrentWaypointIndex];
	const FVector CurrentLocation = GetActorLocation();

	// Direction towards waypoint
	FVector Direction = TargetLocation - CurrentLocation;
	const float DistToWaypoint = Direction.Size();
	Direction.Normalize();

	// Rotate towards movement direction
	if (!Direction.IsNearlyZero())
	{
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
		const FRotator SmoothedRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, 8.f);
		SetActorRotation(FRotator(0.f, SmoothedRotation.Yaw, 0.f));
	}

	// Move using CharacterMovementComponent for proper collision
	const float EffectiveSpeed = Stats.MoveSpeed * CurrentSpeedMultiplier;

	if (UCharacterMovementComponent* MovComp = GetCharacterMovement())
	{
		MovComp->MaxWalkSpeed = EffectiveSpeed;
	}

	AddMovementInput(Direction, 1.f);

	// Track distance for path progress
	const float StepDistance = EffectiveSpeed * DeltaTime;
	DistanceCovered += StepDistance;

	// Check arrival at waypoint
	if (DistToWaypoint <= WaypointAcceptanceRadius)
	{
		AdvanceToNextWaypoint();
	}
}

void AEnemyBase::AdvanceToNextWaypoint()
{
	CurrentWaypointIndex++;

	if (CurrentWaypointIndex >= Waypoints.Num())
	{
		// Reached the final waypoint (the goal)
		HandleReachedGoal();
	}
}

void AEnemyBase::CachePathLength()
{
	TotalPathLength = 0.f;

	for (int32 i = 1; i < Waypoints.Num(); ++i)
	{
		TotalPathLength += FVector::Dist(Waypoints[i - 1], Waypoints[i]);
	}
}

// -----------------------------------------------------------------
// Combat
// -----------------------------------------------------------------

void AEnemyBase::ApplyDamage(float DamageAmount)
{
	if (!IsAlive())
	{
		return;
	}

	// Apply armor reduction
	const float EffectiveDamage = FMath::Max(DamageAmount - Stats.Armor, 1.f);
	Stats.CurrentHealth = FMath::Max(Stats.CurrentHealth - EffectiveDamage, 0.f);

	if (Stats.CurrentHealth <= 0.f)
	{
		HandleDeath();
	}
}

void AEnemyBase::ApplySlow(float SlowMultiplier, float Duration)
{
	CurrentSpeedMultiplier = FMath::Clamp(SlowMultiplier, 0.1f, 1.f);
	SlowTimer = Duration;
}

// -----------------------------------------------------------------
// Events
// -----------------------------------------------------------------

void AEnemyBase::HandleReachedGoal_Implementation()
{
	OnReachedGoal.Broadcast(this);
	Destroy();
}

void AEnemyBase::HandleDeath_Implementation()
{
	OnDeath.Broadcast(this, Stats.GoldReward);
	Destroy();
}

void AEnemyBase::InitializeStats()
{
	Stats.CurrentHealth = Stats.MaxHealth;
}
