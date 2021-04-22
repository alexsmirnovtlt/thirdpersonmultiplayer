// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "General/GameplayStructs.h"
#include "AbilitySystemInterface.h"

#include "ThirdPersonCharacter.generated.h"

enum class ETeamType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnKilledDelegate, AThirdPersonCharacter*, DiedPawn);

UCLASS(abstract)
class TPMULTIPLAYER_API AThirdPersonCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AThirdPersonCharacter(const class FObjectInitializer& ObjectInitializer);
	friend class AMainGameMode;
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void Reset() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
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
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void CurrentRelativeToPawnVelocity(float Axis_X, float Axis_Y); // used in AnimBP to blend legs movement
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void CurrentControllerPitch(float Pitch);

	UFUNCTION(BlueprintNativeEvent, Category = "Override - Third Person Character")
	class USceneComponent* GetShootCheckOrigin();
	
	void AuthPrepareForNewGameRound();

	// GAS related 
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	class UAbilitySystemComponent* AbilitySystemComponent;

	//bool bAbilityInputWasSet;

	//

protected:

	// Animation variables and their replication
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnAnimStateChanged)
	FCharacterAnimState AnimState;
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	const FCharacterAnimState& GetAnimState() const { return AnimState; };
	UFUNCTION(Server, Unreliable)
	void Server_UpdateAnimationState(FCharacterAnimState NewAnimState);
	void Server_UpdateAnimationState_Implementation(FCharacterAnimState NewAnimState) { AnimState = NewAnimState; OnAnimStateChanged(); };
	
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	void ReplicateAnimationStateChange();
	//

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
	float ShootingTimeCooldownMS;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters - Shooting")
	float ReloadTimeCooldownMS;
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
	float LastReloadTime;
	float LastShootingTime;

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
