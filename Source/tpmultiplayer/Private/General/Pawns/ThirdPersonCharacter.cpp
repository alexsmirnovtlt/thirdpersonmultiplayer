// Aleksandr Smirnov 2021


#include "General/Pawns/ThirdPersonCharacter.h"

#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Components/CapsuleComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameState.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagContainer.h"

#include "General/GameplayAbilitySystem/DefaultPawnAttributeSet.h"
#include "General/ActorComponents/TPCMovementComponent.h"
#include "General/Controllers/GamePlayerController.h"


AThirdPersonCharacter::AThirdPersonCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UTPCMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	AimingCameraDistance = FVector(0, 40.f, 0);
	IdleCameraDistance = FVector(0, 70.f, 0);
	AimingCameraSpringDistance = 50.f;
	IdleCameraSpringDistance = 150.f;
	MaxPitch_FreeCamera = 75;
	bViewObstructed = false;
	MaxPitch_Aiming = 60;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 80.f;
	BaseLookUpRate = 80.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 150;
	GetCharacterMovement()->MaxWalkSpeed = 200.f;

	CameraGimbal = CreateDefaultSubobject<USceneComponent>(TEXT("CameraGimbal"));
	CameraGimbal->SetupAttachment(RootComponent);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CameraGimbal);
	CameraBoom->TargetArmLength = IdleCameraSpringDistance;
	CameraBoom->bUsePawnControlRotation = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AIStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AI Stimuli Source Component"));
	// Note that senses must be set in BP. For sound we using UAISense_Hearing::ReportNoiseEvent so we dont need to define it here apparently

	AutoPossessAI = EAutoPossessAI::Disabled;
}

void AThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	AttributeSet = AbilitySystemComponent->GetSet<UDefaultPawnAttributeSet>();

	if (AttributeSet)
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AThirdPersonCharacter::OnHealthAttibuteChanged);
	
	AbilitySystemComponent->RegisterGameplayTagEvent(AimingTagForSimulatedProxies).AddUObject(this, &AThirdPersonCharacter::OnAimStateTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(VIPTag).AddUObject(this, &AThirdPersonCharacter::OnVIPTagChanged);

	// When client joins the game, tag events will not be triggered yet so no VIP tags, dead montage or aiming will be shown. Assuming we already got this tags replicated to us, we need to trigger this manually
	if (!HasAuthority() && AttributeSet)
	{
		OnHealthChanged(AttributeSet->GetMaxHealth(), AttributeSet->GetHealth());
		if( AbilitySystemComponent->HasMatchingGameplayTag(VIPTag)) OnVIPTagChanged(VIPTag, 1);
		if (AbilitySystemComponent->HasMatchingGameplayTag(AimingTagForSimulatedProxies)) OnAimStateTagChanged(AimingTagForSimulatedProxies, 1);
	}
}

void AThirdPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled() && IsPlayerControlled()) // Updating Local player`s camera location and rotation
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
}

void AThirdPersonCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (auto TeamAgentInterface = Cast<IGenericTeamAgentInterface>(NewController))
		TeamAgentInterface->SetGenericTeamId(FGenericTeamId((uint8)TeamType));

	AIStimuliSourceComponent->RegisterWithPerceptionSystem();
	AbilitySystemComponent->InitAbilityActorInfo(NewController, this);
}

void AThirdPersonCharacter::UnPossessed()
{
	UnregisterFromPerceptionSystem();

	Super::UnPossessed(); // important to call it last because GetOwner() in UnregisterFromPerceptionSystem() will return nullptr otherwise
}

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(AGamePlayerController::MoveForwardAxisBindingName, this, &AThirdPersonCharacter::MoveForward);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveRightAxisBindingName, this, &AThirdPersonCharacter::MoveRight);

	PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &AThirdPersonCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &AThirdPersonCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction(AGamePlayerController::SwitchShoulderBindingName, IE_Pressed, this, &AThirdPersonCharacter::SwitchShoulderCamera);

	// Binding Input in project settings with Ability system`s Abilities. No additional setup needed for them to Activate

	FGameplayAbilityInputBinds InputBinds = FGameplayAbilityInputBinds(
		AGamePlayerController::AbilityConfirmBindingName.ToString(),
		AGamePlayerController::AbilityCancelBindingName.ToString(),
		FString("EAbilityInputID"), // enum name
		int32(EAbilityInputID::AbilityConfirm),
		int32(EAbilityInputID::AbilityCancel)
	);

	AbilitySystemComponent->BindAbilityActivationToInputComponent(PlayerInputComponent, InputBinds);
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

	if (Controller->PlayerState && !Controller->PlayerState->IsABot())
		CurrentRotation = Controller->GetControlRotation();
	else
		CurrentRotation = FollowCamera->GetComponentRotation();

	const FRotator ForwardRotation = FRotator(0, CurrentRotation.Yaw, 0);
	const FVector Direction = FRotationMatrix(ForwardRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Direction, Value);
	if (Controller->IsPlayerController()) Controller->SetControlRotation(CameraGimbal->GetComponentRotation());
}

void AThirdPersonCharacter::TurnAtRate(float Value) // Should not be called for AIs 
{
	float AddedYaw = Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds();

	if (IsInAimingAnimation())
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
	
	bool bIsAimingNow = IsInAimingAnimation();

	// Restricting Camera Pitch
	float MaxPitch = bIsAimingNow ? MaxPitch_Aiming : MaxPitch_FreeCamera;
	float MinPitch = bIsAimingNow ? -MaxPitch_Aiming : -MaxPitch_FreeCamera;
	TargetCameraBoomRotation.Pitch = FMath::Clamp(TargetCameraBoomRotation.Pitch, MinPitch, MaxPitch);
	
	CameraBoom->SetRelativeRotation(TargetCameraBoomRotation);

	if (bIsAimingNow)
	{
		auto TargetCntrRot = CameraBoom->GetComponentRotation();
		TargetCntrRot.Pitch += AddedPitch;
		if (Controller) Controller->SetControlRotation(TargetCntrRot);
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

// END Input related logic

bool AThirdPersonCharacter::IsAlive()
{
	if (!AttributeSet) return false;
	return AttributeSet->GetHealth() > 0.f;
}

bool AThirdPersonCharacter::IsVIP()
{
	if (!AbilitySystemComponent) return false;
	return AbilitySystemComponent->HasMatchingGameplayTag(VIPTag);
}

bool AThirdPersonCharacter::HasAmmo()
{
	if (!AttributeSet) return false;
	return AttributeSet->GetAmmoCount() > 0;
}

bool AThirdPersonCharacter::ShootIfAble_Local()
{
	// If no ammo, we still need to replicate shot, but it will just replicate empty mag sound
	if (AttributeSet && AttributeSet->GetAmmoCount() < 1.f)
	{
		FShootData ShootData;
		ShootData.Shooter = this;
		ShootData.Target = nullptr;
		ShootData.bIsClipEmpty = true;
		ShootData.ImpactLocation = FVector::ZeroVector;
		ShootData.ImpactNormal = FVector::ZeroVector;
		ShootData.ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

		if(!HasAuthority()) OnRep_Shot(ShootData); // server will call that in Server_Shoot
		Server_Shoot(ShootData);
		return false;
	}

	// Allow server to execute Shoot Effect (decrease number of bullets) when client shoots but do nothing else
	// and wait for client to receive shooting data and replicate if data is ok.
	if (HasAuthority() && !IsLocallyControlled()) return true;

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
	ShootData.bIsClipEmpty = false;
	ShootData.ImpactLocation = HitResult.Location;
	ShootData.ImpactNormal = HitResult.Normal;
	ShootData.ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	if (!HasAuthority()) OnRep_Shot(ShootData); // Local visualization
	Server_Shoot(ShootData);
	return true;
}

void AThirdPersonCharacter::ReloadWeaponAndReplicate()
{
	if (HasAuthority())
	{
		// Replicate to other clients, skip owner
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			TWeakObjectPtr<APlayerController> WeakPC = *Iterator;
			if (!WeakPC.IsValid()) continue;

			auto PC = WeakPC.Get();
			if (this->GetNetOwningPlayer() == PC->Player) continue;

			// TODO Relevancy check may be added here. Otherwise it looks like NetMulticast except we are skipping owner

			if (!PC->IsLocalController()) CastChecked<AGamePlayerController>(PC)->Client_ReplicateReload(this);
		}
	}

	OnRep_Reload();
}

// BEGIN General logic

float AThirdPersonCharacter::GetCurrentPitch()
{
	if (IsLocallyControlled()) return Controller->GetControlRotation().Pitch;
	else return GetRemotePitchAsFloat();
}

float AThirdPersonCharacter::GetRemotePitchAsFloat()
{
	float UnclampedValue = (float)RemoteViewPitch * 360.0f / 255.0f;
	if (UnclampedValue > 180.f) return UnclampedValue - 360;
	else return UnclampedValue;
}

FVector AThirdPersonCharacter::GetCurrentRelativeToPawnVelocity()
{
	// Calculate current velocity relative to pawn`s forward vector. Will be used at AnimBP`s blend space for movement
	const FVector WorldVelocity = GetCharacterMovement()->Velocity;
	const FVector RelativeVelocity = WorldVelocity.RotateAngleAxis(-RootComponent->GetComponentRotation().Yaw, FVector::UpVector);

	return FVector(RelativeVelocity.Y, RelativeVelocity.X, 0.f);
}

USceneComponent* AThirdPersonCharacter::GetShootCheckOrigin_Implementation()
{
	return RootComponent; // Should be overriden in BP
}

bool AThirdPersonCharacter::IsInAimingAnimation_Implementation()
{
	return false; // Should be overriden in BP
}

void AThirdPersonCharacter::UnregisterFromPerceptionSystem()
{
	AIStimuliSourceComponent->UnregisterFromPerceptionSystem();
}

// END General logic

// BEGIN Ability System related logic

bool AThirdPersonCharacter::StartAiming_LocalPlayer()
{
	if (bViewObstructed) return false; // If local player have an obstacle before him, ability should end immediately
	//if (!Controller->IsLocalPlayerController()) return true; // Skip camera update for non local player

	if (IsPlayerControlled())
	{
		CameraBoom->TargetArmLength = AimingCameraSpringDistance;
		CameraBoom->SetRelativeLocation(AimingCameraDistance);

		auto TargetCntrRot = CameraBoom->GetComponentRotation();
		TargetCntrRot.Pitch = 0;
		TargetCntrRot.Roll = 0;
		GetController<AGamePlayerController>()->SetControlRotation(TargetCntrRot);
	}

	OnAnimStateChanged_Aiming(true);

	return true;

	/*if (bViewObstructed) return false; // If local player have an obstacle before him, ability should end immediately
	
	if (IsPlayerControlled() && IsLocallyControlled())
	{
		CameraBoom->TargetArmLength = AimingCameraSpringDistance;
		CameraBoom->SetRelativeLocation(AimingCameraDistance);

		auto TargetCntrRot = CameraBoom->GetComponentRotation();
		TargetCntrRot.Pitch = 0;
		TargetCntrRot.Roll = 0;
		GetController<AGamePlayerController>()->SetControlRotation(TargetCntrRot);
	}

	OnAnimStateChanged_Aiming(true);
	return true;*/
}

void AThirdPersonCharacter::EndAiming_LocalPlayer()
{
	if (!IsLocallyControlled()) return;

	if (IsPlayerControlled())
	{
		CameraBoom->TargetArmLength = IdleCameraSpringDistance;
		CameraBoom->SetRelativeLocation(IdleCameraDistance);
	}

	OnAnimStateChanged_Aiming(false);
}

void AThirdPersonCharacter::OnHealthAttibuteChanged(const struct FOnAttributeChangeData& Data)
{
	// TODO Maybe add HUD to a local player when his health is below some threshold
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

void AThirdPersonCharacter::OnAimStateTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (IsLocallyControlled()) return;
	bool bIsAimingNow = NewCount > 0;
	OnAnimStateChanged_Aiming(bIsAimingNow);
}

void AThirdPersonCharacter::OnVIPTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	OnVIPStateChanged(NewCount > 0);
}

// END Ability System related logic

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

		// TODO Relevancy check may be added here. Otherwise it looks like NetMulticast except we are skipping owner

		if(!PC->IsLocalController()) CastChecked<AGamePlayerController>(PC)->Client_ReplicateShot(Data);
	}

	if (!Data.bIsClipEmpty)
	{
		// Making noise for AIs to perceive
		UAISense_Hearing::ReportNoiseEvent(this, Data.Shooter->GetActorLocation(), 1.0, ShooterPawn, ShootingSoundDistance);
	}

	// Broadcasting event that will be picked up by GameMode.
	// This allows us to decouple Pawn and GameMode and in theory have separate client build that does not contain Auth GameMode class where all main logic happens
	if (TargetPawn) TargetPawn->OnPawnDamagedEvent.Broadcast(TargetPawn);

	OnRep_Shot(Data);
}

// END Server logic

// IGenericTeamAgentInterface
FGenericTeamId AThirdPersonCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)TeamType);
}