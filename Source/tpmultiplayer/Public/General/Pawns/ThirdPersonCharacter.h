// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"

#include "FMODEvent.h"
#include "General/GameplayStructs.h"

#include "ThirdPersonCharacter.generated.h"

enum class ETeamType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnDamagedDelegate, AThirdPersonCharacter*, DamagedPawn);

UCLASS(abstract)
class TPMULTIPLAYER_API AThirdPersonCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AThirdPersonCharacter(const class FObjectInitializer& ObjectInitializer);
	friend class AMainGameMode;
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(class AController* NewController) override;
	virtual void UnPossessed() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool IsAlive();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool IsVIP();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool HasAmmo();

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	ETeamType TeamType;

	FOnPawnDamagedDelegate OnPawnDamagedEvent;

	UFUNCTION(BlueprintNativeEvent, Category = "Override - Third Person Character")
	bool IsInAimingAnimation();

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Third Person Char - Animation")
	void OnAnimStateChanged_Aiming(bool bIsAiming);
	UFUNCTION(BlueprintImplementableEvent, Category = "Third Person Char - Animation")
	void OnVIPStateChanged(bool bIsVIP);
	UFUNCTION(BlueprintImplementableEvent, Category = "Third Person Char - Animation")
	void OnHealthChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintNativeEvent, Category = "Override - Third Person Character")
	class USceneComponent* GetShootCheckOrigin();

	// BEGIN GAS related 
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	class UAbilitySystemComponent* AbilitySystemComponent;

	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	bool StartAiming_LocalPlayer();
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	void EndAiming_LocalPlayer();

	void OnHealthAttibuteChanged(const struct FOnAttributeChangeData& Data);
	void OnAimStateTagChanged(const struct FGameplayTag CallbackTag, int32 NewCount);
	void OnVIPTagChanged(const struct FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY()
	const class UDefaultPawnAttributeSet* AttributeSet;

	// Tag that will be used on simulated proxies to update Aiming Animation. Tag will be replicated to them when aiming on owning player happens
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	FGameplayTag AimingTagForSimulatedProxies;
	// Same as AimingTag but for Dead AnimMontage
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	FGameplayTag VIPTag;

	// END GAS related 

	// FMOD related
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "FMOD Audio")
	TArray<UFMODEvent*> WeaponFireSounds;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "FMOD Audio")
	UFMODEvent* EmptyWeaponClipSound;
	//

	// BEGIN AI Related

/*protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Perception")
	class UAIPerceptionStimuliSourceComponent* AIStimuliSourceComponent;*/

	// IGenericTeamAgentInterface
private:
	virtual FGenericTeamId GetGenericTeamId() const override;
	// END AI Related

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Effects")
	class UParticleSystem* ShootParticleEffect_Generic;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Effects")
	class UParticleSystem* ShootParticleEffect_Player;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn Animation State")
	float GetCurrentPitch();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn Animation State")
	FVector GetCurrentRelativeToPawnVelocity();

	// END Animation logic

	bool bViewObstructed; // Do we have a space in front of this pawn to enter aiming state

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Shoot(FShootData Data);
	bool Server_Shoot_Validate(FShootData Data);
	void Server_Shoot_Implementation(FShootData Data);

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnRep_Shot(FShootData ShootData);
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnRep_Reload();

	// BEGIN Input Related logic

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	float BaseTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	float BaseLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	float MaxPitch_FreeCamera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	float MaxPitch_Aiming;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	FVector AimingCameraDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	FVector IdleCameraDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	float AimingCameraSpringDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - General")
	float IdleCameraSpringDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Shooting")
	float ForwardDistanceToAbleToAim = 65.f;
	// Basically a raycast distance
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Shooting")
	float ShootingDistance = 10000.f;
	// Max distance to AI to hear a shot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Shooting")
	float ShootingSoundDistance = 3000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* CameraGimbal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	FRotator LastCameraGimbalRotation; // Used to keep player`s camera rotation the same. See Tick() for more details

private:
	float GetRemotePitchAsFloat();

public:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUpAtRate(float Value);
	void TurnAtRate(float Value);
	void SwitchShoulderCamera();
	// Local pawn calculates all info and sends data to a server
	UFUNCTION(BlueprintCallable, Category = "Pawn Ability Handling")
	bool ShootIfAble_Local();
	UFUNCTION(BlueprintCallable, Category = "Pawn Ability Handling")
	void ReloadWeaponAndReplicate();

	// END Input Related logic
};
