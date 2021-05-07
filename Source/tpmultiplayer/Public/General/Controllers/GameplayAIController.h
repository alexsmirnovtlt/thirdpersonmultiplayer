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
UCLASS(hidecategories = ("ActorTick|ComponentTick|Tags|ComponentReplication|Activation|Variable|Cooking|Replication|Actor|Input"))
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

	// BEGIN GAS Related

public:
	UFUNCTION(BlueprintCallable, Category = "Blackboard and GAS")
	void ChangeAbilityState(EAIUsableAbility AbilityEnum, bool bSetActive);

protected:
	// Map of all abilities that are available to AIs
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<EAIUsableAbility, TSubclassOf<class UGameplayAbility>> AbilitiesMap;
	
	// END GAS Related

	// BEGIN AI related
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Custom AI mad Behaviour Tree Settings")
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "Blackboard Keys")
	FName KeyName_MatchState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
	class UBlackboardComponent* AIBBComponent;

	class UAISenseConfig_Sight* SenseConfig_Sight;
	class UAISenseConfig_Hearing* SenseConfig_Hearing;

	//UFUNCTION()
	//void OnPerceptionUpdated(const TArray<AActor*>& Actors);

	// END AI related

protected:
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
