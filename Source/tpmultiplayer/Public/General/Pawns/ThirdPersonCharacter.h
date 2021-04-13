// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThirdPersonCharacter.generated.h"

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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool IsAlive() { return CurrentHealth > 0; };
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn State")
	bool IsAiming() { return bIsAiming; };
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

	void AuthPrepareForNewGameRound();

	bool bIsAiming;

	// BEGIN Input Related logic

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float MaxPitch_FreeCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float MaxPitch_Aiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* CameraGimbal;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUpAtRate(float Value);
	void TurnAtRate(float Value);
	void ShootingMode(float Value);
	void AimingMode(float Value);

	void SwitchShoulderCamera();

	// END Input Related logic
};
