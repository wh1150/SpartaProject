#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "SpartaGameState.h"
#include "Components/ProgressBar.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
ASpartaCharacter::ASpartaCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->bUsePawnControlRotation = true;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	
	NormalSpeed = 250.0f;
	SprintSpeedMultiplier = 2.0f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	MaxHealth = 100.0f;
	Health = MaxHealth;
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &ASpartaCharacter::Move);
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Triggered, this, &ASpartaCharacter::StartJump);
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Completed, this, &ASpartaCharacter::StopJump);
			}
			if (PlayerController->LookAction)
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &ASpartaCharacter::Look);
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Triggered, this, &ASpartaCharacter::StartSprint);
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this, &ASpartaCharacter::StopSprint);
			}
		}
	}
}

void ASpartaCharacter::Move(const FInputActionValue& value) {
	if (!Controller) return;
	const FVector2D MoveInput = value.Get<FVector2D>();
	
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		if (!bIsReverControl)	AddMovementInput(GetActorForwardVector(), MoveInput.X);
		else					AddMovementInput(GetActorForwardVector(), MoveInput.X * -1);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		if (!bIsReverControl)	AddMovementInput(GetActorRightVector(), MoveInput.Y);
		else					AddMovementInput(GetActorRightVector(), MoveInput.Y * -1);
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& value) {
	if (value.Get<bool>())
	{
		Jump();
	}
} 

void ASpartaCharacter::StopJump(const FInputActionValue& value) {
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void ASpartaCharacter::Look(const FInputActionValue& value) {
	const FVector2D LookInput = value.Get<FVector2D>();
	
	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value) {
	if (GetCharacterMovement())	
	{
		if (!bIsSlowing)GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		else GetCharacterMovement()->MaxWalkSpeed = SprintSpeed / 2;
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value) {
	if (GetCharacterMovement())	
	{
		if (!bIsSlowing)	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		else GetCharacterMovement()->MaxWalkSpeed = NormalSpeed / 2;
	}
}

void ASpartaCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateOverheadHP();
}

float ASpartaCharacter::GetHealth() const
{
	return Health;
}

float ASpartaCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
	UpdateOverheadHP();
	
	if (Health <= 0.0f)
	{
		OnDeath();
	}
	return ActualDamage;
}

void ASpartaCharacter::OnDeath() 
{
	if (GetWorld())
	{
		if (ASpartaGameState* SpartaGameState = Cast<ASpartaGameState>(GetWorld()->GetGameState()))
		{
			SpartaGameState->OnGameOver();
		}
	}
}

void ASpartaCharacter::AddHealth(float Amount) 
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}

void ASpartaCharacter::UpdateOverheadHP() 
{
	if (!OverheadWidget)	return;
	
	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance)	return;
	
	if (UProgressBar* HPBar = Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName(TEXT("HPBar"))))
	{
		const float HealthPercent = MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;
		HPBar->SetPercent(HealthPercent);
	}
}

void ASpartaCharacter::Slow(float Duration) 
{
	GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed / 2;
	bIsSlowing = true;
	FTimerHandle SlowTimerHandle;
	GetWorldTimerManager().SetTimer(
		SlowTimerHandle, 
		[this]()
		{
			bIsSlowing = false;
		}, 
		Duration, 
		false
		);
}

void ASpartaCharacter::Reverse(float Duration) 
{
	bIsReverControl = true;
	FTimerHandle ReverseTimerHandle;
	GetWorldTimerManager().SetTimer(
		ReverseTimerHandle, 
		[this]()
		{
			bIsReverControl = false;
		}, 
		Duration, 
		false
		);
}
