// TowerBase.cpp

#include "TowerBase.h"
#include "Kismet/KismetMathLibrary.h"

ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// Tower mesh
	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMesh->SetupAttachment(RootScene);
	TowerMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Range detection sphere
	RangeCollider = CreateDefaultSubobject<USphereComponent>(TEXT("RangeCollider"));
	RangeCollider->SetupAttachment(RootScene);
	RangeCollider->SetSphereRadius(600.f);
	RangeCollider->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RangeCollider->SetGenerateOverlapEvents(true);
	RangeCollider->SetHiddenInGame(true);

	// Fire point (position in Blueprint at the tip of the tower)
	FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
	FirePoint->SetupAttachment(TowerMesh);
	FirePoint->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
}

void ATowerBase::BeginPlay()
{
	Super::BeginPlay();

	// Set range sphere to match stats
	RangeCollider->SetSphereRadius(Stats.AttackRange);

	// Bind overlap events
	RangeCollider->OnComponentBeginOverlap.AddDynamic(this, &ATowerBase::OnRangeBeginOverlap);
	RangeCollider->OnComponentEndOverlap.AddDynamic(this, &ATowerBase::OnRangeEndOverlap);
}

void ATowerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Clean up dead/destroyed enemies from list
	EnemiesInRange.RemoveAll([](const AEnemyBase* E)
		{
			return !IsValid(E) || !E->IsAlive();
		});

	UpdateTarget();

	if (CurrentTarget && IsValid(CurrentTarget) && CurrentTarget->IsAlive())
	{
		RotateTowardsTarget(DeltaTime);

		// Fire rate cooldown
		FireCooldown -= DeltaTime;
		if (FireCooldown <= 0.f)
		{
			FireAtTarget(CurrentTarget);
			FireCooldown = 1.f / FMath::Max(Stats.FireRate, 0.1f);
		}
	}
}

// -----------------------------------------------------------------
// Damage
// -----------------------------------------------------------------

float ATowerBase::GetRandomDamage() const
{
	return FMath::RandRange(Stats.MinDamage, Stats.MaxDamage);
}

// -----------------------------------------------------------------
// Targeting
// -----------------------------------------------------------------

void ATowerBase::OnRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
	{
		if (Enemy->IsAlive())
		{
			EnemiesInRange.AddUnique(Enemy);
		}
	}
}

void ATowerBase::OnRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
	{
		EnemiesInRange.Remove(Enemy);

		if (CurrentTarget == Enemy)
		{
			CurrentTarget = nullptr;
			OnTargetLost();
		}
	}
}

void ATowerBase::UpdateTarget()
{
	// If current target is still valid, keep it
	if (CurrentTarget && IsValid(CurrentTarget) && CurrentTarget->IsAlive()
		&& EnemiesInRange.Contains(CurrentTarget))
	{
		return;
	}

	// Need a new target
	AEnemyBase* OldTarget = CurrentTarget;
	CurrentTarget = SelectBestTarget();

	if (CurrentTarget && CurrentTarget != OldTarget)
	{
		OnTargetAcquired(CurrentTarget);
	}
	else if (!CurrentTarget && OldTarget)
	{
		OnTargetLost();
	}
}

AEnemyBase* ATowerBase::SelectBestTarget() const
{
	if (EnemiesInRange.Num() == 0)
	{
		return nullptr;
	}

	AEnemyBase* Best = nullptr;
	float BestValue = -1.f;

	for (AEnemyBase* Enemy : EnemiesInRange)
	{
		if (!IsValid(Enemy) || !Enemy->IsAlive())
		{
			continue;
		}

		float Value = 0.f;

		switch (TargetMode)
		{
		case ETargetingMode::First:
			// Highest path progress = closest to goal
			Value = Enemy->GetPathProgress();
			break;

		case ETargetingMode::Last:
			// Lowest path progress (invert so higher = better for our comparison)
			Value = 1.f - Enemy->GetPathProgress();
			break;

		case ETargetingMode::Closest:
			// Shortest distance (invert)
		{
			const float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
			Value = Stats.AttackRange - Dist;
		}
		break;

		case ETargetingMode::Strongest:
			// Highest current HP
			Value = Enemy->GetStats().CurrentHealth;
			break;
		}

		if (!Best || Value > BestValue)
		{
			Best = Enemy;
			BestValue = Value;
		}
	}

	return Best;
}

void ATowerBase::RotateTowardsTarget(float DeltaTime)
{
	if (!CurrentTarget)
	{
		return;
	}

	const FVector TargetLoc = CurrentTarget->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLoc);
	const FRotator Smoothed = FMath::RInterpTo(GetActorRotation(), LookAt, DeltaTime, 10.f);
	SetActorRotation(FRotator(0.f, Smoothed.Yaw, 0.f));
}

// -----------------------------------------------------------------
// Virtual Events (override in subclasses / Blueprints)
// -----------------------------------------------------------------

void ATowerBase::FireAtTarget_Implementation(AEnemyBase* Target)
{
	// Base implementation: direct damage (no projectile)
	if (Target)
	{
		Target->ApplyDamage(GetRandomDamage());
	}
}

void ATowerBase::OnTargetAcquired_Implementation(AEnemyBase* Target)
{
	// Override in BP for targeting FX
}

void ATowerBase::OnTargetLost_Implementation()
{
	// Override in BP to stop targeting FX
}