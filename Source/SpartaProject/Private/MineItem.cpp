// Fill out your copyright notice in the Description page of Project Settings.


#include "MineItem.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AMineItem::AMineItem() 
{
	ExplosionDelay = 5.0f;
	ExplosionRadius = 300.0f;
	ExplosionDamage = 30;
	ItemType = "Mine";
	bHasExploded = false;
	
	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
	ExplosionCollision->InitSphereRadius(ExplosionRadius);
	ExplosionCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ExplosionCollision->SetupAttachment(Scene);
}

void AMineItem::ActivateItem(AActor* Activator)
{
	// 타이머 핸들러
	if (bHasExploded)	return;
	Super::ActivateItem(Activator);
	GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &AMineItem::Explode, ExplosionDelay, false);
	bHasExploded = true;
}

void AMineItem::Explode() 
{
	if (ExplosionParticle)
	{
	
		if (UParticleSystemComponent* Particle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation(), GetActorRotation(), false))
		{
			GetWorld()->GetTimerManager().SetTimer(
				DestroyParticleTimerHandle, 
				[Particle]()
				{
					if (Particle && Particle->IsActive())
						Particle->DestroyComponent();
				},
				2.0f, 
				false
			);
		}
	}
	if (ExplosionSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
	}
	
	TArray<AActor*> OverlappingActors;
	ExplosionCollision->GetOverlappingActors(OverlappingActors);
	
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
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

void AMineItem::ResetParticleTimer() 
{
	GetWorld()->GetTimerManager().ClearTimer(ExplosionTimerHandle);
}

FName AMineItem::GetItemType() const
{
	return Super::GetItemType();
}
