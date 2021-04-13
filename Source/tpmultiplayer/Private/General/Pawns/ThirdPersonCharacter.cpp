// Aleksandr Smirnov 2021


#include "General/Pawns/ThirdPersonCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

#include "General/Controllers/GamePlayerController.h"
#include "General/States/GameplayGameState.h"

AThirdPersonCharacter::AThirdPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 80.f;
	BaseLookUpRate = 80.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraGimbal = CreateDefaultSubobject<USceneComponent>(TEXT("CameraGimbal"));
	CameraGimbal->SetupAttachment(RootComponent);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CameraGimbal);
	CameraBoom->TargetArmLength = 150.0f;
	CameraBoom->bUsePawnControlRotation = false; // Enables only when aiming

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AutoPossessAI = EAutoPossessAI::Disabled;
	MaxPitch_FreeCamera = 75;
	MaxPitch_Aiming = 75;
	StartingHealth = 100;
	bIsAiming = false;
	bIsVIP = false;
}

void AThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) CurrentHealth = StartingHealth;
}

void AThirdPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AThirdPersonCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (DamageTaken > 0) CurrentHealth -= DamageTaken;
	OnRep_HealthChanged();

	return DamageTaken;
}

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(AGamePlayerController::MoveForwardAxisBindingName, this, &AThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveRightAxisBindingName, this, &AThirdPersonCharacter::MoveRight);

	PlayerInputComponent->BindAxis(AGamePlayerController::PrimaryActionAxisBindingName, this, &AThirdPersonCharacter::ShootingMode);
	PlayerInputComponent->BindAxis(AGamePlayerController::SecondaryActionAxisBindingName, this, &AThirdPersonCharacter::AimingMode);

	PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &AThirdPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &AThirdPersonCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction(AGamePlayerController::SwitchShoulderBindingName, IE_Pressed, this, &AThirdPersonCharacter::SwitchShoulderCamera);
}

// BEGIN Input related logic

void AThirdPersonCharacter::MoveForward(float Value)
{
	if (!Controller || Value == 0.0) return;

	FRotator CurrentRotation;

	if (Controller->PlayerState && Controller->PlayerState->IsABot())
		CurrentRotation = Controller->GetControlRotation(); // Forward for bots is always theirs ControlRotation
	else
		CurrentRotation = FollowCamera->GetComponentRotation(); // Forward for players is always where theirs camera is looking

	const FRotator ForwardRotation = FRotator(0, CurrentRotation.Yaw, 0);
	const FVector Direction = FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(Direction, Value);
}

void AThirdPersonCharacter::MoveRight(float Value)
{
	if (!Controller || Value == 0.0) return;

	FRotator CurrentRotation;

	if (Controller->PlayerState && Controller->PlayerState->IsABot())
		CurrentRotation = Controller->GetControlRotation();
	else
		CurrentRotation = FollowCamera->GetComponentRotation();

	const FRotator ForwardRotation = FRotator(0, CurrentRotation.Yaw, 0);
	const FVector Direction = FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Direction, Value);
}

void AThirdPersonCharacter::TurnAtRate(float Value) // Should not be called for AIs 
{
	/*if (bIsAiming) AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	else
		CameraGimbal->AddLocalRotation(FRotator(0, Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds(), 0));*/

	float AddedYaw = Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds();

	if (bIsAiming)
	{
		AddControllerYawInput(AddedYaw);
		AddActorLocalRotation(FRotator(0, AddedYaw, 0));
	}
	else
	{
		CameraGimbal->AddLocalRotation(FRotator(0, AddedYaw, 0));
	}
}

void AThirdPersonCharacter::LookUpAtRate(float Value) // Should not be called for AIs 
{
	float AddedPitch = Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds();;

	if (bIsAiming)
	{
		FRotator AddedRotation = FRotator(AddedPitch, 0, 0);

		AddControllerPitchInput(AddedPitch);

		//AddControllerYawInput(Value.X * BaseTurnRate * GetWorld()->GetDeltaSeconds());
		FRotator TargetCameraBoomRotation = CameraBoom->GetRelativeRotation() + AddedRotation;
		// Restricting Camera Pitch
		if (TargetCameraBoomRotation.Pitch > MaxPitch_Aiming) TargetCameraBoomRotation.Pitch = MaxPitch_FreeCamera;
		if (TargetCameraBoomRotation.Pitch < -MaxPitch_Aiming) TargetCameraBoomRotation.Pitch = -MaxPitch_FreeCamera;

		CameraBoom->SetRelativeRotation(TargetCameraBoomRotation);
	}
	else
	{
		FRotator AddedRotation = FRotator(AddedPitch, 0, 0);

		AddControllerPitchInput(AddedPitch);

		//AddControllerYawInput(Value.X * BaseTurnRate * GetWorld()->GetDeltaSeconds());
		FRotator TargetCameraBoomRotation = CameraBoom->GetRelativeRotation() + AddedRotation;
		// Restricting Camera Pitch
		if (TargetCameraBoomRotation.Pitch > MaxPitch_FreeCamera) TargetCameraBoomRotation.Pitch = MaxPitch_FreeCamera;
		if (TargetCameraBoomRotation.Pitch < -MaxPitch_FreeCamera) TargetCameraBoomRotation.Pitch = -MaxPitch_FreeCamera;

		CameraBoom->SetRelativeRotation(TargetCameraBoomRotation);
	}
}

void AThirdPersonCharacter::AimingMode(float Value)
{
	bIsAiming = Value > 0.1f;

	// TODO CHANGE
	if (bIsAiming && CameraBoom->TargetArmLength > 50.f)
		CameraBoom->TargetArmLength = 50.f;
	else if (!bIsAiming && CameraBoom->TargetArmLength < 150.f)
		CameraBoom->TargetArmLength = 150.f;
}

void AThirdPersonCharacter::ShootingMode(float Value)
{
	if (!bIsAiming) return;
}

void AThirdPersonCharacter::SwitchShoulderCamera()
{
	auto CameraLocation = CameraBoom->GetRelativeLocation();
	CameraLocation.Y *= -1;
	CameraBoom->SetRelativeLocation(CameraLocation);
}

// END Input related logic

// BEGIN General logic

void AThirdPersonCharacter::OnRep_HealthChanged()
{
	if (!IsAlive())
	{
		OnPawnKilledEvent.Broadcast(this);
		OnKilled(); // call to BP
	}

	auto GameState = GetWorld()->GetGameState<AGameplayGameState>();
	if (GameState && GameState->GetCurrentMatchData().MatchState == EMatchState::Warmup && CurrentHealth == StartingHealth)
		OnPreparedForNewRound(); // Just started new round, set animations to idle and other custom stuff in BP
}

void AThirdPersonCharacter::OnRep_VIPChanged()
{
	OnVIPChanged(bIsVIP); // Call to BP
}

void AThirdPersonCharacter::AuthPrepareForNewGameRound()
{
	if (!HasAuthority()) return;

	CurrentHealth = StartingHealth;
	OnPreparedForNewRound(); // call to BP
}

// END General logic

void AThirdPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThirdPersonCharacter, bIsVIP);
	DOREPLIFETIME(AThirdPersonCharacter, CurrentHealth);
}