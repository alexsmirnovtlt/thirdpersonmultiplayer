// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirdPersonCharacter.generated.h"

USTRUCT(Blueprintable)
struct FCharacterAnimState
{
	GENERATED_BODY()

	FCharacterAnimState()
	{
		bIsDead = false;
		bIsAiming = false;
		bIsReloading = false;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReloading;
};

enum class ETeamType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnKilledDelegate, AThirdPersonCharacter*, DiedPawn);

UCLASS(abstract)
class TPMULTIPLAYER_API AThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AThirdPersonCharacter();
	friend class AMainGameMode;
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void Reset() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	void AuthPrepareForNewGameRound();
	void ReplicateAnimationStateChange();

	// Animation variables and their replication
	UPROPERTY(ReplicatedUsing=OnAnimStateChanged)
	FCharacterAnimState AnimState;
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	FCharacterAnimState& GetAnimState() { return AnimState; };
	UFUNCTION(Server, Unreliable)
	void Server_UpdateAnimationState(FCharacterAnimState NewAnimState);
	void Server_UpdateAnimationState_Implementation(FCharacterAnimState NewAnimState) { AnimState = NewAnimState; OnAnimStateChanged(); };
	
	UFUNCTION(BlueprintCallable, Category = "Pawn Animation State")
	void ReloadingAnimationEnded();
	
	//

	// BEGIN Input Related logic

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float BaseTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float BaseLookUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float MaxPitch_FreeCamera;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float MaxPitch_Aiming;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	FVector AimingCameraDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	FVector IdleCameraDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float AimingCameraSpringDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float IdleCameraSpringDistance;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float MaxWalkSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float MaxSprintSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom Parameters")
	float ReloadTimeCooldownMS;

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

public:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUpAtRate(float Value);
	void TurnAtRate(float Value);
	void ShootingMode(float Value);
	void AimingMode(float Value);
	void Sprint(float Value);
	void ReloadWeapon();

	void SwitchShoulderCamera();

	// END Input Related logic
};
