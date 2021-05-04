// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayAIController.generated.h"

enum class EMatchState : uint8;

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayAIController : public AAIController
{
	GENERATED_BODY()
public:
	AGameplayAIController();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void OnPossess(class APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	class AGameplayGameState* GameState;

protected:

	// BEGIN GAS Related
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> AimAbility;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> ReloadAbility;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> ShootAbility;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> SprintAbility;

	void CancelAllAbilities();
	// END GAS Related

	UFUNCTION()
	void OnDamaged(class AThirdPersonCharacter* Self);

	UFUNCTION()
	void OnMatchStateChanged();
	EMatchState CurrentMatchState;

	FDelegateHandle MatchStateChangedDelegateHandle;

	// DEBUG
	float DEBUG_DeltaTimePassed;
	bool DEBUG_ClockwiseRotation;
	float DEBUG_RotationSpeed;
	float DEBUG_JumpPeriod;
	float DEBUG_MovementsSpeed;

	float ActionTime = 0.f;
	//
	class AThirdPersonCharacter* PossessedCharacter;
};
