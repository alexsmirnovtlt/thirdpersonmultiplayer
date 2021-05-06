// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "UBTDecorator_IsMatchStateEqualTo.generated.h"

enum class EMatchState : uint8;

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API UUBTDecorator_IsMatchStateEqualTo : public UBTDecorator
{
	GENERATED_UCLASS_BODY()
	
protected:
	UPROPERTY(EditAnywhere, Category = "Match State")
	FBlackboardKeySelector MatchStateKey;
	UPROPERTY(EditAnywhere, Category = "Match State")
	EMatchState MatchState;

public:

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR
};
