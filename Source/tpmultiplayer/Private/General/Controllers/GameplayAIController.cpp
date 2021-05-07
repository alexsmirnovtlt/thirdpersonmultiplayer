// Aleksandr Smirnov 2021


#include "General/Controllers/GameplayAIController.h"

#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardData.h"
#include "Delegates/IDelegateInstance.h"
#include "AbilitySystemComponent.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"
#include "General/GameplayStructs.h"

AGameplayAIController::AGameplayAIController()
{
	AIBBComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("AI Blackboard Component"));

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception Component"));
	
	SenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	SenseConfig_Sight->SightRadius = 3000.f;
	SenseConfig_Sight->LoseSightRadius = 3500.f;
	SenseConfig_Sight->PeripheralVisionAngleDegrees = 90.f;
	SenseConfig_Sight->SetMaxAge(1.f);
	PerceptionComponent->ConfigureSense(*SenseConfig_Sight);
	PerceptionComponent->SetDominantSense(SenseConfig_Sight->GetSenseImplementation());

	SenseConfig_Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	PerceptionComponent->ConfigureSense(*SenseConfig_Hearing);
	SenseConfig_Hearing->HearingRange = 3000.f;
	SenseConfig_Hearing->SetMaxAge(2.f);

	//PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AGameplayAIController::OnPerceptionUpdated);
	
	bNetLoadOnClient = false;
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
	else { UE_LOG(LogTemp, Error, TEXT("AGameplayAIController::OnPossess BehaviorTree was not set!")); }
	
	auto NewTeamId = FGenericTeamId((uint8)PossessedCharacter->TeamType);
	SetGenericTeamId(NewTeamId);

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

// Begin AI logic

/*void AGameplayAIController::OnPerceptionUpdated(const TArray<AActor*>& Actors)
{
	UE_LOG(LogTemp, Warning, TEXT("OnPerceptionUpdated : %d"), Actors.Num());
}*/

// End AI logic

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