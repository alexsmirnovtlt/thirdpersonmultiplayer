// Aleksandr Smirnov 2021


#include "General/Pawns/ThirdPersonCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerState.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "General/GameplayAbilitySystem/DefaultPawnAttributeSet.h"
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
	MaxWalkSpeed = 200.f;
	MaxSprintSpeed = 600.f;
	LastShootingTime = 0.f;
	LastReloadTime = 0.f;
	ReloadTimeCooldownMS = 1.f;
	ShootingTimeCooldownMS = 0.5f;
	MaxPitch_FreeCamera = 75;
	MaxPitch_Aiming = 60;
	StartingHealth = 100;
	bViewObstructed = false;
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

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));

	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) CurrentHealth = StartingHealth;

	//AttributeSet = AbilitySystemComponent->GetSet<UDefaultPawnAttributeSet>();
	//if (!AttributeSet) UE_LOG(LogTemp, Error, TEXT("AThirdPersonCharacter: Incorrect AttributeSet"));
}

void AThirdPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		if (auto PlayerController = GetController<APlayerController>())
		{
			FVector CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation().Vector().GetUnsafeNormal();

			// Check against static geometry if we have space to aim
			FVector StartLocation = GetShootCheckOrigin()->GetComponentLocation();
			FVector EndLocation = StartLocation + CameraRotation * ForwardDistanceToAbleToAim;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);

			bViewObstructed = GetWorld()->LineTraceTestByChannel(StartLocation, EndLocation, ECollisionChannel::ECC_WorldStatic, QueryParams);

			// Keeps Camera Rotation the same regardless of Pawn`s movement
			// Our Camera can only be moved by Player`s Input. But its a part of an actor and will be moved and rotated with it.
			CameraGimbal->SetWorldRotation(LastCameraGimbalRotation);
		}
	}

	// Parameters that will be used by Animation Blueprints
	if (!AnimState.bIsDead)
	{
		// Calculate current velocity relative to pawn`s forward vector. Will be used at AnimBP`s blend space for movement
		const FVector WorldVelocity = GetCharacterMovement()->Velocity;
		const FVector RelativeVelocity = WorldVelocity.RotateAngleAxis(-RootComponent->GetComponentRotation().Yaw, FVector::UpVector);
		CurrentRelativeToPawnVelocity(RelativeVelocity.Y, RelativeVelocity.X); // BP Event
	}
	else CurrentRelativeToPawnVelocity(0, 0);

	if (IsLocallyControlled()) CurrentControllerPitch(Controller->GetControlRotation().Pitch); // Used in AnimBP too
	else CurrentControllerPitch(GetRemotePitchAsFloat());
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
	
	PlayerInputComponent->BindAxis(AGamePlayerController::SprintAxisBindingName, this, &AThirdPersonCharacter::Sprint);

	PlayerInputComponent->BindAction(AGamePlayerController::ReloadBindingName, IE_Pressed, this, &AThirdPersonCharacter::ReloadWeapon);
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
	if (Controller && Controller->IsPlayerController()) Controller->SetControlRotation(CameraGimbal->GetComponentRotation());
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
	if (Controller && Controller->IsPlayerController()) Controller->SetControlRotation(CameraGimbal->GetComponentRotation());
}

void AThirdPersonCharacter::TurnAtRate(float Value) // Should not be called for AIs 
{
	float AddedYaw = Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds();

	if (AnimState.bIsAiming)
	{
		auto TargetCntrRot = CameraBoom->GetComponentRotation();
		if(Controller) Controller->SetControlRotation(TargetCntrRot);
	}
	
	CameraGimbal->AddLocalRotation(FRotator(0, AddedYaw, 0));
	LastCameraGimbalRotation = CameraGimbal->GetComponentRotation();
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

	if (AnimState.bIsAiming)
	{
		auto TargetCntrRot = CameraBoom->GetComponentRotation();
		TargetCntrRot.Pitch += AddedPitch;
		if (Controller) Controller->SetControlRotation(TargetCntrRot);
	}
}

void AThirdPersonCharacter::AimingMode(float Value)
{
	bool IsAimingNow = Value > 0.7f; // could be anything > 0
	if (bViewObstructed) IsAimingNow = false;
	bool StateChanged = false;

	if (!AnimState.bIsAiming && IsAimingNow) // Start to aim
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
	else if (AnimState.bIsAiming && !IsAimingNow) // End aiming
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

void AThirdPersonCharacter::SwitchShoulderCamera()
{
	auto NewCameraBoomLocation = CameraBoom->GetRelativeLocation();
	auto NewCameraBoomRotation = CameraBoom->GetRelativeRotation();

	NewCameraBoomLocation.Y *= -1;
	NewCameraBoomRotation.Yaw *= -1;

	CameraBoom->SetRelativeLocation(NewCameraBoomLocation);
	CameraBoom->SetRelativeRotation(NewCameraBoomRotation);
}

void AThirdPersonCharacter::Sprint(float Value)
{
	// TODO Add sprint in GAS

	//if (Value > 0.5f) GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void AThirdPersonCharacter::ShootingMode(float Value)
{
	if (Value < 0.5f || !AnimState.bIsAiming || AnimState.bIsReloading) return;
	
	// TODO make unable to shoot if hands are in anim transition between idle/aiming

	float CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	if (CurrentTime - LastShootingTime < ShootingTimeCooldownMS) return;

	LastShootingTime = CurrentTime;
	ReplicateAnimationStateChange();

	// Gathering info about what we are about to hit

	FVector CameraRotation;
	
	if (auto PlayerController = GetController<APlayerController>())
		CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation().Vector().GetUnsafeNormal();
	else CameraRotation = GetActorForwardVector();

	FHitResult HitResult = FHitResult();
	FVector StartLocation = GetShootCheckOrigin()->GetComponentLocation();
	FVector EndLocation = StartLocation + CameraRotation * ShootingDistance;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bValidHit = GetWorld()->LineTraceSingleByProfile(HitResult, StartLocation, EndLocation, TEXT("Destructible"), QueryParams); // Meshes have that preset by default

	AActor* TargetActor = nullptr;
	if (HitResult.Actor.IsValid())
		TargetActor = Cast<AThirdPersonCharacter>(HitResult.Actor.Get());

	// Setting up info about our hit to send to server
	FShootData ShootData;

	ShootData.Shooter = this;
	ShootData.Target = TargetActor;
	ShootData.bIsValidHit = bValidHit;
	ShootData.ImpactLocation = HitResult.Location;
	ShootData.ImpactNormal = HitResult.Normal;
	ShootData.ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	OnRep_Shot(ShootData); // Local visualization
	Server_Shoot(ShootData);
}

void AThirdPersonCharacter::ReloadWeapon()
{
	if (AnimState.bIsReloading) return;

	float CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	if (CurrentTime - LastReloadTime < ReloadTimeCooldownMS) return;

	LastReloadTime = CurrentTime;
	AnimState.bIsReloading = true;
	ReplicateAnimationStateChange();
}

// END Input related logic

float AThirdPersonCharacter::GetRemotePitchAsFloat()
{
	float UnclampedValue = (float)RemoteViewPitch * 360.0f / 255.0f;
	if (UnclampedValue > 180.f) return UnclampedValue - 360;
	else return UnclampedValue;
}

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

USceneComponent* AThirdPersonCharacter::GetShootCheckOrigin_Implementation()
{
	return RootComponent; // Should be overriden in BP
}

// BEGIN Shooting Server and Client logic

bool AThirdPersonCharacter::Server_Shoot_Validate(FShootData Data)
{
	// TODO Add Validation that player really hit that actor and was not cheating
	// For that:
	// - We need to store all pawn`s locations for a period of time (usually a few seconds total, lets say 30 snapshots per second). Better place for this can be a GameMode
	// - Find closest stored location`s timestamp for shooter and target using FShootData.ServerTime
	// - Spawn test capsules and raycast to check if hit could have really happened

	return true;
}

void AThirdPersonCharacter::Server_Shoot_Implementation(FShootData Data)
{
	if (!Data.Shooter) return;
	auto ShooterPawn = CastChecked<AThirdPersonCharacter>(Data.Shooter);
	AThirdPersonCharacter* TargetPawn = nullptr;

	if(Data.Target) TargetPawn = CastChecked<AThirdPersonCharacter>(Data.Target);

	// Replicating Shot for every client except shooter himself
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		TWeakObjectPtr<APlayerController> WeakPC = *Iterator;
		if (!WeakPC.IsValid()) continue;

		auto PC = WeakPC.Get();
		if (Data.Shooter->GetNetOwningPlayer() == PC->Player) continue;

		CastChecked<AGamePlayerController>(PC)->Client_ReplicateShot(Data);
	}

	if (TargetPawn)
	{
		FDamageEvent DamageEvent;
		TargetPawn->TakeDamage(DamagePerShot, DamageEvent, nullptr, Data.Shooter);
	}
}

// END Server logic

void AThirdPersonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThirdPersonCharacter, bIsVIP);
	DOREPLIFETIME(AThirdPersonCharacter, CurrentHealth);
	DOREPLIFETIME_CONDITION(AThirdPersonCharacter, AnimState, COND_SkipOwner);
}