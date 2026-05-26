// ArrowProjectile.cpp

#include "ArrowProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

AArrowProjectile::AArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(15.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	// Mesh (assign arrow mesh in Blueprint)
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Projectile movement (optional, we do manual homing in Tick)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->InitialSpeed = 0.f; // We control speed manually
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bAutoActivate = false; // We use manual Tick movement

	InitialLifeSpan = 5.f;
}

void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AArrowProjectile::OnOverlap);

	SetLifeSpan(MaxLifetime);
}

void AArrowProjectile::InitProjectile(AEnemyBase* InTarget, float InDamage)
{
	TargetEnemy = InTarget;
	Damage = InDamage;
}

void AArrowProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasHit)
	{
		return;
	}

	MoveTowardsTarget(DeltaTime);
}

void AArrowProjectile::MoveTowardsTarget(float DeltaTime)
{
	FVector TargetLocation;

	if (IsValid(TargetEnemy) && TargetEnemy->IsAlive())
	{
		// Aim at the center of the enemy
		TargetLocation = TargetEnemy->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	}
	else
	{
		// Target died or was destroyed, keep flying forward and auto-destroy
		const FVector Forward = GetActorForwardVector();
		SetActorLocation(GetActorLocation() + Forward * Speed * DeltaTime);
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	FVector Direction = TargetLocation - CurrentLocation;
	const float Distance = Direction.Size();
	Direction.Normalize();

	// Rotate arrow to face target
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
	SetActorRotation(LookAt);

	// Move towards target
	const FVector NewLocation = CurrentLocation + Direction * Speed * DeltaTime;
	SetActorLocation(NewLocation);

	// Direct hit check (in case overlap doesn't fire at high speeds)
	if (Distance <= Speed * DeltaTime + 30.f)
	{
		if (IsValid(TargetEnemy) && TargetEnemy->IsAlive())
		{
			TargetEnemy->ApplyDamage(Damage);
		}
		DestroyProjectile();
	}
}

void AArrowProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasHit)
	{
		return;
	}

	AEnemyBase* HitEnemy = Cast<AEnemyBase>(OtherActor);
	if (!HitEnemy || !HitEnemy->IsAlive())
	{
		return;
	}

	// Only damage the intended target (or any enemy if target is lost)
	if (HitEnemy == TargetEnemy || !IsValid(TargetEnemy))
	{
		HitEnemy->ApplyDamage(Damage);
		DestroyProjectile();
	}
}

void AArrowProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	DestroyProjectile();
}

void AArrowProjectile::DestroyProjectile()
{
	bHasHit = true;
	// You can spawn hit particles here in a Blueprint override
	Destroy();
}