// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UItemInterface : public UInterface
{
	GENERATED_BODY()
};

class SPARTAPROJECT_API IItemInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION()
	virtual void OnItemOverlap(
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult) = 0; // 오버랩 트리거
	UFUNCTION()
	virtual void OnItemEndOverlap(
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex) = 0; // 오버랩 엔드 트리거
	virtual void ActivateItem(AActor* Activator) = 0; // 아이템 발동
	virtual FName GetItemType() const = 0; // 아이템 타입 가져오기
	
	//지뢰, 힐링, 코인
	// 힐링, 코인 - 즉시 발동
	// 지뢰 - 범위 내 오버랩 > 5초 뒤 폭발 > 5초동안 오버랩중이면 데미지
};
