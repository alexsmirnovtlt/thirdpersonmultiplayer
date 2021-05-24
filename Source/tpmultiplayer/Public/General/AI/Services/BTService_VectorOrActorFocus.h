// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_VectorOrActorFocus.generated.h"

/**
 * Slighntly modified version of UBTService_DefaultFocus at has two blackboard keys instead of one
 */
UCLASS(hidecategories = (Service))
class TPMULTIPLAYER_API UBTService_VectorOrActorFocus : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:

	UBTService_VectorOrActorFocus(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual uint16 GetInstanceMemorySize() const override { return 24; } // 24 is Sizeof FBTFocusMemory defined in UBTService_DefaultFocus. Better to include header then doing this
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual FString GetStaticDescription() const override { return "Either Focus visible actor or last heard shot location"; };

	EBlackboardNotificationResult OnBlackboardKeyValueChange_Actor(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);
	EBlackboardNotificationResult OnBlackboardKeyValueChange_Vector(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID);

	UPROPERTY(EditAnywhere, Category = Blackboard)
	struct FBlackboardKeySelector BlackboardKey_Vector;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR
};
