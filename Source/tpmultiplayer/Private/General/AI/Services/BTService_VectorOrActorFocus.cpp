// Aleksandr Smirnov 2021


#include "General/AI/Services/BTService_VectorOrActorFocus.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Services/BTService_DefaultFocus.h" // for the struct
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "AIController.h"


UBTService_VectorOrActorFocus::UBTService_VectorOrActorFocus(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Keep Updating Aiming Target";

	bNotifyTick = false;
	bTickIntervals = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	// accept only actors and vectors
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_VectorOrActorFocus, BlackboardKey), AActor::StaticClass());
	BlackboardKey_Vector.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_VectorOrActorFocus, BlackboardKey_Vector));
}

void UBTService_VectorOrActorFocus::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	UBlackboardData* BBAsset = GetBlackboardAsset();
	BlackboardKey.ResolveSelectedKey(*BBAsset);
	BlackboardKey_Vector.ResolveSelectedKey(*BBAsset);
}

void UBTService_VectorOrActorFocus::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	FBTFocusMemory* MyMemory = (FBTFocusMemory*)NodeMemory;
	check(MyMemory);
	MyMemory->Reset();

	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();

	auto KeyID_Actor = BlackboardKey.GetSelectedKeyID();
	auto KeyID_Vector = BlackboardKey_Vector.GetSelectedKeyID();

	MyBlackboard->RegisterObserver(KeyID_Actor, this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_VectorOrActorFocus::OnBlackboardKeyValueChange_Actor));
	MyBlackboard->RegisterObserver(KeyID_Vector, this, FOnBlackboardChangeNotification::CreateUObject(this, &UBTService_VectorOrActorFocus::OnBlackboardKeyValueChange_Vector));
}

void UBTService_VectorOrActorFocus::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	FBTFocusMemory* MyMemory = (FBTFocusMemory*)NodeMemory;
	check(MyMemory);
	AAIController* OwnerController = OwnerComp.GetAIOwner();

	if (OwnerController != NULL)
	{
		bool bClearFocus = false;
		if (MyMemory->bActorSet) bClearFocus = (MyMemory->FocusActorSet == OwnerController->GetFocusActorForPriority(EAIFocusPriority::Default));
		else bClearFocus = (MyMemory->FocusLocationSet == OwnerController->GetFocalPointForPriority(EAIFocusPriority::Default));

		if (bClearFocus) OwnerController->ClearFocus(EAIFocusPriority::Default);
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp) BlackboardComp->UnregisterObserversFrom(this);
}

EBlackboardNotificationResult UBTService_VectorOrActorFocus::OnBlackboardKeyValueChange_Actor(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	AAIController* OwnerController = OwnerComp ? OwnerComp->GetAIOwner() : nullptr;
	if (OwnerController == nullptr) return EBlackboardNotificationResult::ContinueObserving;

	const int32 NodeInstanceIdx = OwnerComp->FindInstanceContainingNode(this);
	FBTFocusMemory* MyMemory = (FBTFocusMemory*)OwnerComp->GetNodeMemory(this, NodeInstanceIdx);
	MyMemory->Reset();
	OwnerController->ClearFocus(EAIFocusPriority::Default);

	UObject* KeyValue = Blackboard.GetValue<UBlackboardKeyType_Object>(ChangedKeyID);
	AActor* TargetActor = Cast<AActor>(KeyValue);
	if (TargetActor)
	{
		OwnerController->SetFocus(TargetActor, EAIFocusPriority::Default);
		MyMemory->FocusActorSet = TargetActor;
		MyMemory->bActorSet = true;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

EBlackboardNotificationResult UBTService_VectorOrActorFocus::OnBlackboardKeyValueChange_Vector(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	AAIController* OwnerController = OwnerComp ? OwnerComp->GetAIOwner() : nullptr;
	if (OwnerController == nullptr) return EBlackboardNotificationResult::ContinueObserving;

	const int32 NodeInstanceIdx = OwnerComp->FindInstanceContainingNode(this);
	FBTFocusMemory* MyMemory = (FBTFocusMemory*)OwnerComp->GetNodeMemory(this, NodeInstanceIdx);

	if (MyMemory->bActorSet) return EBlackboardNotificationResult::ContinueObserving; // Ignore if have enemy in sight

	MyMemory->Reset();
	OwnerController->ClearFocus(EAIFocusPriority::Default);

	const FVector FocusLocation = Blackboard.GetValue<UBlackboardKeyType_Vector>(ChangedKeyID);
	OwnerController->SetFocalPoint(FocusLocation, EAIFocusPriority::Default);
	MyMemory->FocusLocationSet = FocusLocation;

	return EBlackboardNotificationResult::ContinueObserving;
}

#if WITH_EDITOR

FName UBTService_VectorOrActorFocus::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Service.DefaultFocus.Icon");
}

#endif	// WITH_EDITOR