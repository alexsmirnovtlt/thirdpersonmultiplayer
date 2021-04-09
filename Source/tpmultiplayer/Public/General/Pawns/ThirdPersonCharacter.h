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
	bool IsAlive();
	bool HasFlag() { return bHasFlag; };

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void MoveForward(float Value);
	void MoveRight(float Value);

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

	UPROPERTY(ReplicatedUsing = OnRep_FlagOwnerChanged)
	bool bHasFlag;
	UFUNCTION()
	void OnRep_FlagOwnerChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnKilled();
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnPreparedForNewRound();
	UFUNCTION(BlueprintImplementableEvent, Category = "Pawn Events")
	void OnFlagOwnershipChanged(bool GotFlag);

	void AuthPrepareForNewGameRound();

	// ! TMP CHECK

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	//  ! TMP CHECK


};
