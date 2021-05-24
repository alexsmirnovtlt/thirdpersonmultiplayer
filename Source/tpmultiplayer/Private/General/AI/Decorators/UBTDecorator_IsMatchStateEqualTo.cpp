// Aleksandr Smirnov 2021


#include "General/AI/Decorators/UBTDecorator_IsMatchStateEqualTo.h"

#include "UObject/Class.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"

#include "General/GameplayStructs.h"


UUBTDecorator_IsMatchStateEqualTo::UUBTDecorator_IsMatchStateEqualTo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Is MatchState Equal To";

	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	MatchState = EMatchState::Warmup;
}

void UUBTDecorator_IsMatchStateEqualTo::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset) MatchStateKey.ResolveSelectedKey(*BBAsset);
}

bool UUBTDecorator_IsMatchStateEqualTo::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return false;

	auto MatchStateAsUint = BlackboardComp->GetValue<UBlackboardKeyType_Enum>(MatchStateKey.GetSelectedKeyID());
	return (EMatchState)MatchStateAsUint == MatchState;
}

FString UUBTDecorator_IsMatchStateEqualTo::GetStaticDescription() const
{
	const TEnumAsByte<EMatchState> MatchStateAsByte = MatchState;
	FString MatchStateStr = UEnum::GetValueAsString(MatchStateAsByte.GetValue());
	return FString::Printf(TEXT("%s: %s"), *Super::GetStaticDescription(), *MatchStateStr);
}

void UUBTDecorator_IsMatchStateEqualTo::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->RegisterObserver(MatchStateKey.GetSelectedKeyID(), this, FOnBlackboardChangeNotification::CreateUObject(this, &UUBTDecorator_IsMatchStateEqualTo::OnBlackboardKeyValueChange));
	}
}

void UUBTDecorator_IsMatchStateEqualTo::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp) BlackboardComp->UnregisterObserversFrom(this);
}

EBlackboardNotificationResult UUBTDecorator_IsMatchStateEqualTo::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* BehaviorComp = static_cast<UBehaviorTreeComponent*>(Blackboard.GetBrainComponent());
	
	if (BehaviorComp == nullptr) return EBlackboardNotificationResult::RemoveObserver;
	else if (MatchStateKey.GetSelectedKeyID() == ChangedKeyID) BehaviorComp->RequestExecution(this);

	return EBlackboardNotificationResult::ContinueObserving;
}

#if WITH_EDITOR

FName UUBTDecorator_IsMatchStateEqualTo::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Decorator.CompareBlackboardEntries.Icon");
}

#endif	// WITH_EDITOR