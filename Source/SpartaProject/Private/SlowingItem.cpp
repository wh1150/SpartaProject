// Fill out your copyright notice in the Description page of Project Settings.


#include "SlowingItem.h"

#include "SpartaCharacter.h"

ASlowingItem::ASlowingItem() {
	Duration = 5.0f;
	ItemType = "Slowing";
}

void ASlowingItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
	if (TSubclassOf<ASpartaCharacter>(Activator->GetClass()) && Cast<ASpartaCharacter>(Activator))
	{
		Cast<ASpartaCharacter>(Activator)->Slow(Duration);
	}
	
	DestroyItem();
	
}
