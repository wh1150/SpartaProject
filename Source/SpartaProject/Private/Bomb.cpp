// Fill out your copyright notice in the Description page of Project Settings.


#include "Bomb.h"

#include "SpartaCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ABomb::ABomb() 
{
	
}

void ABomb::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(
		this,
		&ABomb::Explode
	);
}

void ABomb::Explode() {
	
	Super::ActivateItem(nullptr);
	if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), 
			ExplosionParticle, 
			GetActorLocation(), 
			GetActorRotation(), 
			true
		);
	}
	if (ExplosionSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	TArray<AActor*> OverlappingActors;
	Collision->GetOverlappingActors(OverlappingActors);
	
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			FVector Direction = Actor->GetActorLocation() - this->GetActorLocation();
			float Distance = Direction.Size();

			Direction = Direction.GetSafeNormal();

			float Alpha = 1.0f - FMath::Clamp(Distance / ExplosionRadius, 0.0f, 1.0f);
			float LaunchStrength = FMath::Lerp(100.0f, 2000.0f, Alpha);
			Cast<ASpartaCharacter>(Actor)->LaunchCharacter(
				Direction * LaunchStrength,
				false,
				false
			);
			UGameplayStatics::ApplyDamage(
				Actor,
				ExplosionDamage,
				nullptr,
				this,
				UDamageType::StaticClass()
			);
		}
	} 
	DestroyItem();
}

FName ABomb::GetItemType() const
{
	return Super::GetItemType();
}
