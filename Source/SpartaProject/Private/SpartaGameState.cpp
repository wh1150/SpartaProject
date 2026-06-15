// Fill out your copyright notice in the Description page of Project Settings.


#include "SpartaGameState.h"

#include "CoinItem.h"
#include "MineItem.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpawnVolume.h"
#include "SpikeTrap.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

ASpartaGameState::ASpartaGameState()
{
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	Score = 0;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
	CurrentWaveIndex = 0;
	MaxWaves = 3;
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();
	StartLevel();
	
	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);
}

void ASpartaGameState::StartLevel() 
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->ShowGameHUD();
		}
	}
	if (UGameInstance* Src = GetGameInstance())
	{
		if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(Src))
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}
	StartWave();
}

void ASpartaGameState::StartWave() 
{
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	
	// 현재 스폰된 스폰 볼륨을 가져온다.
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	if (FoundVolumes.Num() > 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Wave %d 시작!"), CurrentWaveIndex+1));
		if ( ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]) )
		{
			ActiveSpawnVolume = SpawnVolume;
			// 40개의 아이템 액터를 랜덤 스폰시킨다.
			const int32 ItemToSpawn = 10 + CurrentWaveIndex * 10;
			for (int32 i = 0; i < ItemToSpawn; i++){
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}	
			
			if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GetGameInstance()))
			{
				if (SpartaGameInstance->CurrentWaveIndex == 1)
				{
					for (int32 i = 0; i < 10; i++){
						SpawnVolume->SpawnRandomTrap();
					}		
				}
				if (SpartaGameInstance->CurrentWaveIndex == 2)
				{
					GetWorldTimerManager().SetTimer(
						BombSpawnTimerHandle,
						this,
						&ASpartaGameState::SpawnBomb,
						0.2f,
						true
					);
				}
			}
		}
		// 스폰이 완료되면 30초 뒤에 EndWave 호출 
		GetWorldTimerManager().SetTimer(
			WaveTimerHandle,
			this,
			&ASpartaGameState::EndWave,
			LevelWaveDuration[CurrentWaveIndex],
			false
		);
	}
}

void ASpartaGameState::EndWave() 
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(BombSpawnTimerHandle);
	
	// 현재 스폰된 아이템들을 가져와 삭제한다.
	TArray<AActor*> FoundItems;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseItem::StaticClass(), FoundItems);
	
	for (AActor* FoundItem : FoundItems) {
		if(TSubclassOf<ABaseItem>(FoundItem->GetClass()))
		{
			Cast<ABaseItem>(FoundItem)->ResetParticleTimer();
			if (TSubclassOf<AMineItem>(FoundItem->GetClass()))
			{
				Cast<AMineItem>(FoundItem)->ResetParticleTimer();
			}
		}
		FoundItem->Destroy();
	}
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpikeTrap::StaticClass(), FoundItems);
	
	for (AActor* FoundItem : FoundItems) {
		if(TSubclassOf<ABaseItem>(FoundItem->GetClass()))
		{
			Cast<ABaseItem>(FoundItem)->ResetParticleTimer();
			if (TSubclassOf<AMineItem>(FoundItem->GetClass()))
			{
				Cast<AMineItem>(FoundItem)->ResetParticleTimer();
			}
		}
		FoundItem->Destroy();
	}
	
	
	
	AddScore(Score);
	if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GetGameInstance()))
	{
		CurrentWaveIndex++;
		SpartaGameInstance->CurrentWaveIndex = CurrentWaveIndex;
	}
	if (CurrentWaveIndex >= MaxWaves)	EndLevel();
	else								StartWave();
}

void ASpartaGameState::EndLevel()
{
	// 타이머 초기화
	if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GetGameInstance()))
	{
		CurrentLevelIndex++;
		SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;
	}
	
	// 방금 끝난 레벨이 마지막 레벨이었으면 게임 종료
	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver();
		return;
	}
	// 다음 레벨이 존재하면 다음 레벨을 오픈한다.
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))	UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	else												OnGameOver();
}

void ASpartaGameState::OnGameOver() 
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->SetPause(true);
			SpartaPlayerController->ShowMainMenu(true);
		}
	}
}

int32 ASpartaGameState::GetScore() const
{
	return Score;
}

// 코인 획득시 호출
void ASpartaGameState::AddScore(int32 Amount) 
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

// 코인 획득시 호출2
void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Collected %d / %d"), CollectedCoinCount, SpawnedCoinCount);
	
	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		EndWave();
	}
}

void ASpartaGameState::UpdateHUD() 
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
				}
				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance))
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
						}
					}
				}
				if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d-%d"), CurrentLevelIndex+1, CurrentWaveIndex+1)));
				}
			}
		}
	}
}
void ASpartaGameState::SpawnBomb()
{
	if (!ActiveSpawnVolume)
	{
		return;
	}

	ActiveSpawnVolume->SpawnRandomBomb();
}