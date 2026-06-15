// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "Bomb.generated.h"

/**
 * 
 */
UCLASS()
class SPARTAPROJECT_API ABomb : public ABaseItem
{
	GENERATED_BODY()

public:
	ABomb();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ExplosionRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ExplosionDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	UParticleSystem* ExplosionParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	USoundBase* ExplosionSound;
	
	void BeginPlay() override;
	
protected:
	
	
	void Explode();
	virtual FName GetItemType() const override;
};
