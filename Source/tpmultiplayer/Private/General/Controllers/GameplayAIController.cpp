// Aleksandr Smirnov 2021


#include "General/Controllers/GameplayAIController.h"

#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "General/GameModes/MainGameMode.h"
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

	SenseConfig_Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	SenseConfig_Hearing->HearingRange = 3000.f;
	SenseConfig_Hearing->SetMaxAge(2.f);

	PerceptionComponent->ConfigureSense(*SenseConfig_Sight);
	PerceptionComponent->ConfigureSense(*SenseConfig_Hearing);
	PerceptionComponent->SetDominantSense(SenseConfig_Sight->GetSenseImplementation());

	PerceptionComponent->OnTargetPerceptionInfoUpdated.AddDynamic(this, &AGameplayAIController::OnTargetPerceptionUpdated);

	bNetLoadOnClient = false;
}

void AGameplayAIController::BeginPlay()
{
	Super::BeginPlay();

	GameState = GetWorld()->GetGameState<AGameplayGameState>();
}

void AGameplayAIController::OnPossess(class APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessedCharacter = Cast<AThirdPersonCharacter>(InPawn);
	if (!PossessedCharacter || !GameState) { UE_LOG(LogTemp, Error, TEXT("AGameplayAIController::OnPossess no PossessedCharacter or GameState!")); return; }

	if (BehaviorTree) RunBehaviorTree(BehaviorTree);
	else { UE_LOG(LogTemp, Error, TEXT("AGameplayAIController::OnPossess BehaviorTree was not set!")); }

	OnMatchStateChanged();

	MatchStateChangedDelegateHandle = GameState->OnMatchDataChanged().AddUObject(this, &AGameplayAIController::OnMatchStateChanged);
	PossessedCharacter->OnPawnDamagedEvent.AddDynamic(this, &AGameplayAIController::OnDamaged);
}

void AGameplayAIController::OnUnPossess()
{
	GameState->OnMatchDataChanged().Remove(MatchStateChangedDelegateHandle);
	
	if (PossessedCharacter)
	{
		PossessedCharacter->OnPawnDamagedEvent.RemoveDynamic(this, &AGameplayAIController::OnDamaged);
		PossessedCharacter->GetAbilitySystemComponent()->CancelAllAbilities();
	}

	Super::OnUnPossess();
}

void AGameplayAIController::OnMatchStateChanged()
{
	if (!GameState || !Blackboard) return;

	auto& MatchData = GameState->GetCurrentMatchData();
	CurrentMatchState = MatchData.MatchState;

	// Updating blackboards values. Depending on a matchstate, we may reset or update some blackboard values
	Blackboard->SetValueAsEnum(KeyName_MatchState, (uint8)MatchData.MatchState);

	bool AreaCaptureInProgress = MatchData.SpecialMessage == EInGameSpecialMessage::AreaCaptureInProgress;
	Blackboard->SetValueAsBool(KeyName_IsAreaCaptureInProgress, AreaCaptureInProgress);

	if (MatchData.MatchState == EMatchState::Warmup)
	{
		Blackboard->SetValueAsObject(KeyName_VisibleEnemy, nullptr);
		Blackboard->SetValueAsVector(KeyName_LastHeardShot, FAISystem::InvalidLocation);
	}
	else if (MatchData.MatchState == EMatchState::Gameplay)
	{
		Blackboard->SetValueAsBool(KeyName_IsVIP, PossessedCharacter ? PossessedCharacter->IsVIP() : false);
	}
}

void AGameplayAIController::OnDamaged(class AThirdPersonCharacter* Self)
{
	if (PossessedCharacter && !PossessedCharacter->IsAlive())
	{
		PossessedCharacter->GetAbilitySystemComponent()->CancelAllAbilities();
	}	
}

// Begin AI logic

void AGameplayAIController::OnTargetPerceptionUpdated(const FActorPerceptionUpdateInfo& UpdateInfo)
{
	// Shooting sounds that server makes are all considered as Enemy for everyone (probably because we calling UAISense_Hearing::ReportNoiseEvent), so we may want to skip execution if shot was made by a friendly pawn
	// TODO Fix that issue

	if (CurrentMatchState != EMatchState::Gameplay) return; // We dont care about any new data if not in a main game state

	if (UpdateInfo.Stimulus.Type == SenseConfig_Sight->GetSenseID())
	{
		if (!UpdateInfo.Target.IsValid()) return;
		UObject* CurrentVisibleActor = Blackboard->GetValueAsObject(KeyName_VisibleEnemy);

		if (UpdateInfo.Stimulus.WasSuccessfullySensed())
		{
			if (CurrentVisibleActor != nullptr)
			{
				// Change actor only if he is closer
				if (!PossessedCharacter) return;
				FVector ActorLocation = PossessedCharacter->GetActorLocation();
				FVector CurrentTargetActorLocation = Cast<AActor>(CurrentVisibleActor)->GetActorLocation();
				FVector NewTargetActorLocation = Cast<AActor>(UpdateInfo.Target)->GetActorLocation();
	
				auto CurrentDistanceToTarget = FVector::DistSquared2D(ActorLocation, CurrentTargetActorLocation);
				auto DistanceToNewTarget = FVector::DistSquared2D(ActorLocation, NewTargetActorLocation);

				if (DistanceToNewTarget < CurrentDistanceToTarget) Blackboard->SetValueAsObject(KeyName_VisibleEnemy, UpdateInfo.Target.Get());
			}
			else Blackboard->SetValueAsObject(KeyName_VisibleEnemy, UpdateInfo.Target.Get());
		}
		else if(UpdateInfo.Target == CurrentVisibleActor) Blackboard->SetValueAsObject(KeyName_VisibleEnemy, nullptr);
	}
	else if (UpdateInfo.Stimulus.Type == SenseConfig_Hearing->GetSenseID())
	{
		// Basically now we are skipping every sense update if target is in the same team, which may not be preffered logic. Look for TODO above for explanation
		// Actually this workaround can be skipped if we WANT AIs to always face last heard shot(if no actual enemy in sight of course), no matter its from ally or enemy. More realistic?
		auto TargetAsTeamInterface = Cast<IGenericTeamAgentInterface>(UpdateInfo.Target);
		if (TargetAsTeamInterface && TargetAsTeamInterface->GetGenericTeamId() == GetGenericTeamId()) return;

		FVector CurrentLastHeardShot = Blackboard->GetValueAsVector(KeyName_LastHeardShot);

		if (UpdateInfo.Stimulus.WasSuccessfullySensed())
		{
			// New sound, update it
			if(!FAISystem::IsValidLocation(CurrentLastHeardShot)) Blackboard->SetValueAsVector(KeyName_LastHeardShot, UpdateInfo.Stimulus.StimulusLocation);
			else
			{
				// Change value only if that shot is closer
				if (!PossessedCharacter) return;
				FVector ActorLocation = PossessedCharacter->GetActorLocation();
				auto CurrentDistanceToSound = FVector::DistSquared2D(ActorLocation, CurrentLastHeardShot);
				auto DistanceToNewSound = FVector::DistSquared2D(ActorLocation, UpdateInfo.Stimulus.ReceiverLocation);

				if(DistanceToNewSound < CurrentDistanceToSound) Blackboard->SetValueAsVector(KeyName_LastHeardShot, UpdateInfo.Stimulus.StimulusLocation);
			}
		}
		else
		{
			// Resetting Blackboard value only if the same sound location was reported 
			if(UpdateInfo.Stimulus.StimulusLocation == CurrentLastHeardShot) Blackboard->SetValueAsVector(KeyName_LastHeardShot, FAISystem::InvalidLocation);
		}
	}
}

ETeamAttitude::Type AGameplayAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	// At some points, when player Unpossesses a pawn (to spectate) and before AI possesses it, we have no PossessedCharacter and GetGenericTeamId() returns 255. 
	// It makes new AI that will possess it either hostile or indifferent to everyone. (Attacks teammates)
	// TODO Find where and how to change it. It should not be happening
	
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (!OtherTeamAgent) ETeamAttitude::Neutral;

	if (!PossessedCharacter || GetGenericTeamId().GetId() == 255)
	{
		// Catching that or else this pawn considers everyone an enemy
		// TODO this check should be removed and reworked

		uint8 PresumedTeamType = GetWorld()->GetAuthGameMode<AMainGameMode>()->GetTeamTypeForNewController(this);
		
		if ((uint8)OtherTeamAgent->GetGenericTeamId() == PresumedTeamType) return ETeamAttitude::Friendly;
		else return ETeamAttitude::Hostile;
	}

	if (GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId()) return ETeamAttitude::Friendly;
	return ETeamAttitude::Hostile;
}

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