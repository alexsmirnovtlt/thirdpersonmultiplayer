// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayAIController.generated.h"

enum class EMatchState : uint8;
enum class EAIUsableAbility : uint8;

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
	// Map of all abilities that are available to AIs
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<EAIUsableAbility, TSubclassOf<class UGameplayAbility>> AbilitiesMap;

public:
	UFUNCTION(BlueprintCallable, Category = "Blackboard and GAS")
	void ChangeAbilityState(EAIUsableAbility AbilityEnum, bool bSetActive);
	//void CancelAllAbilities();
	// END GAS Related

	// BEGIN Blackboard and BTree related
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
	FName KeyName_MatchState;

	// END Blackboard and BTree related

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Blackboard")
	class UBlackboardComponent* AIBBComponent;

	void ChangeAbilityState();

	UFUNCTION()
	void OnDamaged(class AThirdPersonCharacter* Self);

	UFUNCTION()
	void OnMatchStateChanged();

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
