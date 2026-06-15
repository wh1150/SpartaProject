// Fill out your copyright notice in the Description page of Project Settings.


#include "SpikeTrap.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ASpikeTrap::ASpikeTrap()
{
	SpikeDamage = 20.0f;
	bHasStab = false;
	bIsUp = true;
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->SetupAttachment(Scene);
	
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Scene);
	
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ASpikeTrap::OnItemOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ASpikeTrap::OnItemEndOverlap);
}

void ASpikeTrap::BeginPlay()
{
	Super::BeginPlay();
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			bIsUp = !bIsUp;
			if (bIsUp)	bHasStab = false;
		},
		2.0f,
		true
	);
}

void ASpikeTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bIsUp)
	{
		SpikeUp(DeltaSeconds);
	}
	else
	{
		SpikeDown(DeltaSeconds);
	}
}

void ASpikeTrap::OnItemOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	if (bHasStab) return;
	if (OtherActor && OtherActor->ActorHasTag("Player"))
	{
		ActivateItem(OtherActor);
		bHasStab = true;
	}
}

void ASpikeTrap::ActivateItem(AActor* Activator) 
{
	TArray<AActor*> OverlappingActors;
	Collision->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			UGameplayStatics::ApplyDamage(
				Actor,
				SpikeDamage,
				nullptr,
				this,
				UDamageType::StaticClass()
			);
		}
	} 
}

FName ASpikeTrap::GetItemType() const
{
	return ItemType;
}
void ASpikeTrap::SpikeUp(float DeltaTime) 
{
	FVector ActorLocation = GetActorLocation();
	if (ActorLocation.Z < -19.0f)
	{
		AddActorLocalOffset(FVector(0.0f, 0.0f, 700.0f * DeltaTime));
	}
}

void ASpikeTrap::SpikeDown(float DeltaTime) 
{
	FVector ActorLocation = GetActorLocation();
	if (ActorLocation.Z > -160.0f)
	{
		AddActorLocalOffset(FVector(0.0f, 0.0f, -700.0f * DeltaTime));
	}
}

void ASpikeTrap::OnItemEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
}
