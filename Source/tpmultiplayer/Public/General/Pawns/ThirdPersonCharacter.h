// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
//#include "GameplayCueInterface.h"
#include "GameFramework/Character.h"
#include "General/GameplayStructs.h"

#include "ThirdPersonCharacter.generated.h"

enum class ETeamType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnKilledDelegate, AThirdPersonCharacter*, DiedPawn);

UCLASS(abstract)
class TPMULTIPLAYER_API AThirdPersonCharacter : public ACharacter, public IAbilitySystemInterface//, public IGameplayCueInterface
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
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PossessedBy(class AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool IsAlive() { return CurrentHealth > 0; };
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool IsVIP() { return bIsVIP; };

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	ETeamType TeamType;

	FOnPawnKilledDelegate OnPawnKilledEvent;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float StartingHealth;

	UPROPERTY(ReplicatedUsing=OnRep_HealthChanged)
	float CurrentHealth;
	UFUNCTION()
	void OnRep_HealthChanged();

	UPROPERTY(ReplicatedUsing = OnRep_VIPChanged)
	bool bIsVIP;
	UFUNCTION()
	void OnRep_VIPChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnKilled();
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnPreparedForNewRound();
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnVIPChanged(bool IsVIP);
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnAnimStateChanged();
	UFUNCTION(BlueprintImplementableEvent, Category = "Third Person Char - Animation")
	void OnAnimStateChanged_Aiming(bool bIsAiming);
	UFUNCTION(BlueprintImplementableEvent, Category = "Third Person Char - Animation")
	void OnAnimStateChanged_IsDead(bool bIsDead);

	UFUNCTION(BlueprintNativeEvent, Category = "Override - Third Person Character")
	class USceneComponent* GetShootCheckOrigin();
	UFUNCTION(BlueprintNativeEvent, Category = "Override - Third Person Character")
	bool IsInAimingAnimation();

	void AuthPrepareForNewGameRound();

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
	void OnDeadStateTagChanged(const struct FGameplayTag CallbackTag, int32 NewCount);

	// Tag that will be used on simulated proxies to update Aiming Animation. Tag will be replicated to them when aiming on owning player happens
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	FGameplayTag AimingTagForSimulatedProxies;
	// Same as AimingTag but for Dead AnimMontage
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	FGameplayTag DeadTagForSimulatedProxies;

	// Gameplay Cue interface
/*public:
	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) { UE_LOG(LogTemp, Warning, TEXT("1!")); };
	virtual void HandleGameplayCues(UObject* Self, const FGameplayTagContainer& GameplayCueTags, EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) {};
	virtual bool ShouldAcceptGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) { return true; };

	//	Deprecated
	virtual void HandleGameplayCue(AActor* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) { UE_LOG(LogTemp ,Warning, TEXT("%s"), *this->GetFName().ToString()); };
	virtual void HandleGameplayCues(AActor* Self, const FGameplayTagContainer& GameplayCueTags, EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) {};
	virtual bool ShouldAcceptGameplayCue(AActor* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) { return true; };
	// Deprecated
	
	virtual void GameplayCueDefaultHandler(EGameplayCueEvent::Type EventType, FGameplayCueParameters Parameters) {};
	virtual void ForwardGameplayCueToParent() {};*/
	//

	// END GAS related 

protected:

	// BEGIN Animation logic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnAnimStateChanged)
	FCharacterAnimState AnimState;
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	const FCharacterAnimState& GetAnimState() const { return AnimState; };
	UFUNCTION(Server, Unreliable)
	void Server_UpdateAnimationState(FCharacterAnimState NewAnimState);
	void Server_UpdateAnimationState_Implementation(FCharacterAnimState NewAnimState) { AnimState = NewAnimState; OnAnimStateChanged(); };
	
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	void ReplicateAnimationStateChange();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn Animation State")
	float GetCurrentPitch();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn Animation State")
	FVector GetCurrentRelativeToPawnVelocity();

	FDelegateHandle AimTagChangedDelegateHandle;

	// END Animation logic

	bool bViewObstructed; // Do we have a space in front of this pawn to enter aiming state

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Shoot(FShootData Data);
	bool Server_Shoot_Validate(FShootData Data);
	void Server_Shoot_Implementation(FShootData Data);
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnRep_Shot(FShootData ShootData);

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Shooting")
	float ShootingDistance = 10000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Shooting")
	float DamagePerShot = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Effects")
	class UParticleSystem* ShootParticleEffect_Generic;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Effects")
	class UParticleSystem* ShootParticleEffect_Player;

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
	//void ShootingMode(float Value);
	//void AimingMode(float Value);

	void SwitchShoulderCamera();

	// END Input Related logic
};
