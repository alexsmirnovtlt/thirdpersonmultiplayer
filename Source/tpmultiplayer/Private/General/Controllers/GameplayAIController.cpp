// Aleksandr Smirnov 2021


#include "General/Controllers/GameplayAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Delegates/IDelegateInstance.h"
#include "AbilitySystemComponent.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"
#include "General/GameplayStructs.h"

AGameplayAIController::AGameplayAIController()
{
	AIBBComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("AI Blackboard Component"));
}

void AGameplayAIController::BeginPlay()
{
	Super::BeginPlay();

	DEBUG_ClockwiseRotation = FMath::SRand() > 0.5f;
	DEBUG_JumpPeriod = FMath::RandRange(0.2f, 0.8f);
	DEBUG_MovementsSpeed = FMath::RandRange(0.3f, 1.f);
	DEBUG_RotationSpeed = FMath::RandRange(0.2f, 1.3f);
}

void AGameplayAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGameplayAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn() || !PossessedCharacter) return;
}

void AGameplayAIController::OnPossess(class APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessedCharacter = Cast<AThirdPersonCharacter>(InPawn);
	if (!PossessedCharacter || !GameState) return;

	OnMatchStateChanged();
	MatchStateChangedDelegateHandle = GameState->OnMatchDataChanged().AddUObject(this, &AGameplayAIController::OnMatchStateChanged);
	
	if(BehaviorTree) RunBehaviorTree(BehaviorTree);
	else { UE_LOG(LogTemp, Error, TEXT("AGameplayAIController::OnPossess Could not sse BT!")); }

	PossessedCharacter->OnPawnDamagedEvent.AddDynamic(this, &AGameplayAIController::OnDamaged);
}

void AGameplayAIController::OnUnPossess()
{
	Super::OnUnPossess();

	GameState->OnMatchDataChanged().Remove(MatchStateChangedDelegateHandle);

	// TODO Stop BT?

	if (PossessedCharacter)
	{
		PossessedCharacter->OnPawnDamagedEvent.RemoveDynamic(this, &AGameplayAIController::OnDamaged);
		PossessedCharacter->GetAbilitySystemComponent()->CancelAllAbilities();
	}
}

void AGameplayAIController::OnMatchStateChanged()
{
	if (!GameState) return;

	auto& MatchData = GameState->GetCurrentMatchData();
	
	if (Blackboard) Blackboard->SetValueAsEnum(KeyName_MatchState, (uint8)MatchData.MatchState);
}

void AGameplayAIController::OnDamaged(class AThirdPersonCharacter* Self)
{
	// DEBUG, check health first
	PossessedCharacter->GetAbilitySystemComponent()->CancelAllAbilities();
}

// Begin Action Logic that be called from Behavior Tree

void AGameplayAIController::ChangeAbilityState(EAIUsableAbility AbilityEnum, bool bSetActive)
{
	if (!GetPawn() || !PossessedCharacter) return;
	if (!AbilitiesMap.Contains(AbilityEnum)) { UE_LOG(LogTemp, Warning, TEXT("AGameplayAIController::ChangeAbilityState trying to change state of ability that is not present in a Map!")); return; }

	if(bSetActive)
		PossessedCharacter->GetAbilitySystemComponent()->TryActivateAbilityByClass(AbilitiesMap[AbilityEnum], false);
	else
		PossessedCharacter->GetAbilitySystemComponent()->CancelAbility(AbilitiesMap[AbilityEnum].GetDefaultObject());
}

// END Action Logic that be called from Behavior Tree