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

	UFUNCTION()
	void OnMatchStateChanged();
	EMatchState CurrentMatchState;

	// DEBUG
	float DEBUG_DeltaTimePassed;
	bool DEBUG_ClockwiseRotation;
	float DEBUG_RotationSpeed;
	float DEBUG_JumpPeriod;
	float DEBUG_MovementsSpeed;
	//
	class AThirdPersonCharacter* PossessedCharacter;
};
