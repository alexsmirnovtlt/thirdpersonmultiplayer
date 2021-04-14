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

	AimingCameraDistance = FVector(0, 40.f, 0);
	IdleCameraDistance = FVector(0, 70.f, 0);
	AnimState = FCharacterAnimState();
	AimingCameraSpringDistance = 50.f;
	IdleCameraSpringDistance = 150.f;
	MaxPitch_FreeCamera = 75;
	MaxPitch_Aiming = 60;
	StartingHealth = 100;
	bIsVIP = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 80.f;
	BaseLookUpRate = 80.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraGimbal = CreateDefaultSubobject<USceneComponent>(TEXT("CameraGimbal"));
	CameraGimbal->SetupAttachment(RootComponent);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CameraGimbal);
	CameraBoom->TargetArmLength = IdleCameraSpringDistance;
	CameraBoom->bUsePawnControlRotation = false; // Enables only when aiming

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) CurrentHealth = StartingHealth;
}

void AThirdPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AnimState.bIsDead)
	{
		// Calculate current velocity relative to pawn`s forward vector. Will be used at AnimBP`s blend space for movement
		const FVector WorldVelocity = GetCharacterMovement()->Velocity;
		const FVector RelativeVelocity = WorldVelocity.RotateAngleAxis(-RootComponent->GetComponentRotation().Yaw, FVector::UpVector);
		CurrentRelativeToPawnVelocity(RelativeVelocity.Y, RelativeVelocity.X); // BP Event
	}
	else CurrentRelativeToPawnVelocity(0, 0);

	if (Controller) CurrentControllerPitch(Controller->GetControlRotation().Pitch); // Used in AnimBP too
}

float AThirdPersonCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (DamageTaken > 0) CurrentHealth -= DamageTaken;

	OnRep_HealthChanged();

	return DamageTaken;
}

void AThirdPersonCharacter::Reset()
{
	Super::Reset();
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
	float AddedYaw = Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds();

	if (AnimState.bIsAiming)
	{
		//
		/*auto TargetCntrRot = CameraBoom->GetComponentRotation();
		TargetCntrRot.Pitch = 0;
		TargetCntrRot.Roll = 0;
		GetController<AGamePlayerController>()->SetControlRotation(TargetCntrRot);*/
		
		AddControllerYawInput(AddedYaw);
	}
	else
	{
		CameraGimbal->AddLocalRotation(FRotator(0, AddedYaw, 0));
	}
}

void AThirdPersonCharacter::LookUpAtRate(float Value) // Should not be called for AIs 
{
	float AddedPitch = Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds();;
	FRotator AddedRotation = FRotator(AddedPitch, 0, 0);
	FRotator TargetCameraBoomRotation = CameraBoom->GetRelativeRotation() + AddedRotation;
	
	// Restricting Camera Pitch
	float MaxPitch = AnimState.bIsAiming ? MaxPitch_Aiming : MaxPitch_FreeCamera;
	float MinPitch = AnimState.bIsAiming ? -MaxPitch_Aiming : -MaxPitch_FreeCamera;
	TargetCameraBoomRotation.Pitch = FMath::Clamp(TargetCameraBoomRotation.Pitch, MinPitch, MaxPitch);
	
	CameraBoom->SetRelativeRotation(TargetCameraBoomRotation);

	// TODO update Controller Pitch so it can be used in AnimBP
	//AddControllerPitchInput(AddedPitch);
}

void AThirdPersonCharacter::AimingMode(float Value)
{
	bool IsAimingNow = Value > 0.7f; // could be anything > 0
	bool StateChanged = false;

	if (!AnimState.bIsAiming && IsAimingNow)
	{
		CameraBoom->TargetArmLength = AimingCameraSpringDistance;
		CameraBoom->SetRelativeLocation(AimingCameraDistance);
		//
		auto TargetCntrRot = CameraBoom->GetComponentRotation();
		TargetCntrRot.Pitch = 0;
		TargetCntrRot.Roll = 0;
		GetController<AGamePlayerController>()->SetControlRotation(TargetCntrRot);
		//
		StateChanged = true;
	}
	else if (AnimState.bIsAiming && !IsAimingNow)
	{
		CameraBoom->TargetArmLength = IdleCameraSpringDistance;
		CameraBoom->SetRelativeLocation(IdleCameraDistance);
		StateChanged = true;
	}

	if (StateChanged)
	{
		AnimState.bIsAiming = IsAimingNow;
		ReplicateAnimationStateChange();
	}
}

void AThirdPersonCharacter::ShootingMode(float Value)
{
	if (!AnimState.bIsAiming || AnimState.bIsReloading) return;

	// TODO Make a shot, preferably on a server
	
	// TODO Add shoot particles and damage particles
}

void AThirdPersonCharacter::SwitchShoulderCamera()
{
	auto NewCameraBoomLocation = CameraBoom->GetRelativeLocation();
	auto NewCameraBoomRotation = CameraBoom->GetRelativeRotation();

	NewCameraBoomLocation.Y *= -1;
	NewCameraBoomRotation.Yaw *= -1;

	CameraBoom->SetRelativeLocation(NewCameraBoomLocation);
	CameraBoom->SetRelativeRotation(NewCameraBoomRotation);
}

// END Input related logic

// BEGIN General logic

void AThirdPersonCharacter::OnRep_HealthChanged()
{
	if (!IsAlive())
	{
		OnPawnKilledEvent.Broadcast(this);
		OnKilled(); // call to BP	

		if (!HasAuthority() && IsLocallyControlled())
		{
			// Special case when pawn owner will not receive updated AnimState bacause of the replication condition so it need to change it for itself too
			AnimState.bIsDead = true; 
			OnAnimStateChanged();
		}

		return;
	}

	auto GameState = GetWorld()->GetGameState<AGameplayGameState>();
	if (GameState && GameState->GetCurrentMatchData().MatchState == EMatchState::Warmup && CurrentHealth == StartingHealth)
	{
		// Just started new round, set animations to idle and other custom stuff in BP
		if (!HasAuthority() && IsLocallyControlled())
		{
			AnimState = FCharacterAnimState(); // Same logic as special case before, local authonomous proxy need to reset its state manually
			OnAnimStateChanged();
		}

		OnPreparedForNewRound();
	}
		
}

void AThirdPersonCharacter::OnRep_VIPChanged()
{
	OnVIPChanged(bIsVIP); // Call to BP
}

void AThirdPersonCharacter::AuthPrepareForNewGameRound()
{
	if (!HasAuthority()) return;

	AnimState = FCharacterAnimState(); // Resetting and replicating animation variables
	ReplicateAnimationStateChange();

	CurrentHealth = StartingHealth; // Resetting and replicating health
	
	OnPreparedForNewRound(); // call to BP
}

void AThirdPersonCharacter::ReplicateAnimationStateChange()
{
	OnAnimStateChanged(); // Auth will replicate FCharacterAnimState without any additional code, but need to call OnRep function on itself
	if (!HasAuthority()) Server_UpdateAnimationState(AnimState);
}

// END General logic

void AThirdPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThirdPersonCharacter, bIsVIP);
	DOREPLIFETIME(AThirdPersonCharacter, CurrentHealth);
	DOREPLIFETIME_CONDITION(AThirdPersonCharacter, AnimState, COND_SkipOwner);
}