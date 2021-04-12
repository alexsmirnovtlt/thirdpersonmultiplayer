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

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 60.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraGimbal = CreateDefaultSubobject<USceneComponent>(TEXT("CameraGimbal"));
	CameraGimbal->SetupAttachment(RootComponent);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CameraGimbal);
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Enables only when aiming

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AutoPossessAI = EAutoPossessAI::Disabled;
	StartingHealth = 100;
	bIsVIP = false;
	bIsAiming = false;
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

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &AGameplaySpectatorPawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &AGameplaySpectatorPawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveForwardAxisBindingName, this, &AThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveRightAxisBindingName, this, &AThirdPersonCharacter::MoveRight);
	//PlayerInputComponent->BindAxis(AGamePlayerController::PrimaryActionAxisBindingName, this, &AThirdPersonCharacter::MoveUp_World);
	//PlayerInputComponent->BindAxis(AGamePlayerController::SecondaryActionAxisBindingName, this, &AThirdPersonCharacter::MoveDown_World);
	
	PlayerInputComponent->BindAxis(AGamePlayerController::SecondaryActionAxisBindingName, this, &AThirdPersonCharacter::AimingMode);

	PlayerInputComponent->BindAction(AGamePlayerController::AdditionalActionBindingName, IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(AGamePlayerController::AdditionalActionBindingName, IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &AThirdPersonCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis("TurnRate", this, &AThirdPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &AThirdPersonCharacter::LookUpAtRate);
	//PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &APawn::AddControllerPitchInput);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &AThirdPersonCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction(AGamePlayerController::SwitchShoulderBindingName, IE_Pressed, this, &AThirdPersonCharacter::SwitchShoulderCamera);
}

float AThirdPersonCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (DamageTaken > 0) CurrentHealth -= DamageTaken;
	OnRep_HealthChanged();

	return DamageTaken;
}

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

void AThirdPersonCharacter::OnRep_FlagOwnerChanged()
{
	OnFlagOwnershipChanged(bIsVIP);
}

void AThirdPersonCharacter::AuthPrepareForNewGameRound()
{
	if (!HasAuthority()) return;
	
	CurrentHealth = StartingHealth;
	OnPreparedForNewRound(); // call to BP
}

// BEGIN Input related logic

void AThirdPersonCharacter::MoveForward(float Value)
{
	if (!Controller || Value == 0.0) return;

	FRotator YawRotation;

	// find out which way is forward
	if (Controller->PlayerState && Controller->PlayerState->IsABot())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		YawRotation = FRotator(0, Rotation.Yaw, 0);
	}
	else
	{
		const FRotator Rotation = FollowCamera->GetComponentRotation();
		YawRotation = FRotator(0, Rotation.Yaw, 0);
	}

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void AThirdPersonCharacter::MoveRight(float Value)
{
	if (!Controller || Value == 0.0) return;

	FRotator YawRotation;

	// find out which way is forward
	if (Controller->PlayerState && Controller->PlayerState->IsABot())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		YawRotation = FRotator(0, Rotation.Yaw, 0);
	}
	else
	{
		const FRotator Rotation = FollowCamera->GetComponentRotation();
		YawRotation = FRotator(0, Rotation.Yaw, 0);
	}

	// get right vector 
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// add movement in that direction
	AddMovementInput(Direction, Value);
}

void AThirdPersonCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());

	if (!bIsAiming)
		CameraGimbal->AddLocalRotation(FRotator(0, Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds(), 0));
}

void AThirdPersonCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());

	if(!bIsAiming)
		CameraBoom->AddLocalRotation(FRotator(-Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds(), 0, 0));
}

void AThirdPersonCharacter::AimingMode(float Value)
{
	bIsAiming = Value > 0.1f;

	if (bIsAiming)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		CameraBoom->TargetArmLength = 50.f;
		CameraBoom->bUsePawnControlRotation = true;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		CameraBoom->TargetArmLength = 150.f;
		CameraBoom->bUsePawnControlRotation = false;
	}
}

void AThirdPersonCharacter::SwitchShoulderCamera()
{
	auto CameraLocation = CameraBoom->GetRelativeLocation();
	CameraLocation.Y *= -1;
	CameraBoom->SetRelativeLocation(CameraLocation);
}

// END Input related logic

bool AThirdPersonCharacter::IsAlive()
{
	return CurrentHealth > 0;
}

void AThirdPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThirdPersonCharacter, bIsVIP);
	DOREPLIFETIME(AThirdPersonCharacter, CurrentHealth);
}