// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
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
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard Keys")
	FName KeyName_VisibleEnemy;
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard Keys")
	FName KeyName_LastHeardShot;
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard Keys")
	FName KeyName_IsVIP;
	UPROPERTY(EditDefaultsOnly, Category = "Blackboard Keys")
	FName KeyName_IsAreaCaptureInProgress;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
	class UBlackboardComponent* AIBBComponent;

	class UAISenseConfig_Sight* SenseConfig_Sight;
	class UAISenseConfig_Hearing* SenseConfig_Hearing;

	UFUNCTION()
	void OnTargetPerceptionUpdated(const struct FActorPerceptionUpdateInfo& UpdateInfo);

	// END AI related

protected:
	UFUNCTION()
	void OnDamaged(class AThirdPersonCharacter* Self);

	UFUNCTION()
	void OnMatchStateChanged();

	FDelegateHandle MatchStateChangedDelegateHandle;
	EMatchState CurrentMatchState;

	class AThirdPersonCharacter* PossessedCharacter;
};
