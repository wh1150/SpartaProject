// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"

#include "SpikeTrap.h"
#include "Bomb.h"
#include "Components/BoxComponent.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);
	ItemDataTable = nullptr;
}

AActor* ASpawnVolume::SpawnRandomItem() 
{
	if (FItemSpawnRow* SelectedRow = GetRandomItem())
	{
		if (UClass* ActualClass = SelectedRow->ItemClass.Get())
		{
			return SpawnItem(ActualClass);
		}
	}
	return nullptr;
}

AActor* ASpawnVolume::SpawnRandomTrap()
{
	if (!SpikeTrapClass)
	{
		return nullptr;
	}
	FVector Location = GetRandomPointInVolume();
	Location.Z = -170.0f;
	ASpikeTrap* SpawnedActor = GetWorld()->SpawnActor<ASpikeTrap>(
		SpikeTrapClass, 
		Location,
		FRotator::ZeroRotator);
	return SpawnedActor;
}

FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
	if (!ItemDataTable) return nullptr;
	
	TArray<FItemSpawnRow*> AllRows;
	
	static const FString ContextString(TEXT("ItemSpawnContext"));
	ItemDataTable->GetAllRows(ContextString, AllRows);
	
	if (AllRows.IsEmpty()) return nullptr;
	
	float TotalChance = 0.0f;
	for (const FItemSpawnRow* Row : AllRows)
	{
		TotalChance += Row->SpawnChance;
	}
	
	const float RandomValue = FMath::FRandRange(0.0f, TotalChance);
	float AccumulateChance = 0.0f;
	
	for (FItemSpawnRow* Row : AllRows)
	{
		AccumulateChance += Row->SpawnChance;
		if (RandomValue <= AccumulateChance)
		{
			return Row;
		}
	}// pendingKill;
	
	return nullptr;
}

FVector ASpawnVolume::GetRandomPointInVolume() const 
{
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	FVector BoxOrigin = SpawningBox->GetComponentLocation();
	
	return BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::FRandRange(-BoxExtent.Z, BoxExtent.Z)
	);
}

AActor* ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass) 
{
	if (!ItemClass) return nullptr;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		ItemClass, 
		GetRandomPointInVolume(),
		FRotator::ZeroRotator);

	return SpawnedActor;
}

AActor* ASpawnVolume::SpawnRandomBomb()
{
	if (!BombClass)
	{
		return nullptr;
	}

	FVector Location = GetRandomPointInVolume();
	Location.Z = 0.0f;
	return GetWorld()->SpawnActor<ABomb>(
		BombClass,
		Location,
		FRotator::ZeroRotator
	);
}
