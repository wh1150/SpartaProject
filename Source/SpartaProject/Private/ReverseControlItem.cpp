// Fill out your copyright notice in the Description page of Project Settings.


#include "ReverseControlItem.h"

#include "SpartaCharacter.h"

AReverseControlItem::AReverseControlItem() {
	Duration = 5.0f;
	ItemType = "ReverseControl";
}

void AReverseControlItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);
	if (TSubclassOf<ASpartaCharacter>(Activator->GetClass()) && Cast<ASpartaCharacter>(Activator))
	{
		Cast<ASpartaCharacter>(Activator)->Reverse(Duration);
	}
	
	DestroyItem();
}
