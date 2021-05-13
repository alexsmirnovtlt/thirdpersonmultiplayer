// Aleksandr Smirnov 2021


#include "General/Controllers/GameplayAIController.h"

#include "Perception/AISenseConfig_Hearing.h"
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

	PerceptionComponent->OnTargetPerceptionInfoUpdated.AddDynamic(this, &AGameplayAIController::OnTargetPerceptionUpdated);

	CurrentMatchState = EMatchState::Gameplay; // so it will pass check in OnMatchStateChanged() initially

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

	if (Blackboard) // will update AreaCapture state that may happen a lot on EMatchState::Gameplay
	{
		bool AreaCaptureInProgress = MatchData.SpecialMessage == EInGameSpecialMessage::AreaCaptureInProgress;
		Blackboard->SetValueAsBool(KeyName_IsAreaCaptureInProgress, AreaCaptureInProgress);
	}
	if (CurrentMatchState == MatchData.MatchState) return; // will not update anything else twice
	
	CurrentMatchState = MatchData.MatchState;

	if (Blackboard)
	{
		// Updating blackboards values. Depending on a matchstate, we may reset or update some blackboard values
		Blackboard->SetValueAsEnum(KeyName_MatchState, (uint8)MatchData.MatchState);

		if (MatchData.MatchState == EMatchState::Warmup)
		{
			// Some keys must be reset on warmup
			Blackboard->SetValueAsObject(KeyName_VisibleEnemy, nullptr);
			Blackboard->SetValueAsVector(KeyName_LastHeardShot, FAISystem::InvalidLocation);
			if (PossessedCharacter) Blackboard->SetValueAsBool(KeyName_IsVIP, PossessedCharacter->IsVIP());
		}
		else if (MatchData.MatchState == EMatchState::Gameplay)
		{
			Blackboard->SetValueAsBool(KeyName_IsVIP, PossessedCharacter->IsVIP());
		}
		else if (MatchData.MatchState == EMatchState::RoundEnd)
		{
			
		}
	}
}

void AGameplayAIController::OnDamaged(class AThirdPersonCharacter* Self)
{
	// DEBUG, check health first
	PossessedCharacter->GetAbilitySystemComponent()->CancelAllAbilities();
}

// Begin AI logic

void AGameplayAIController::OnTargetPerceptionUpdated(const FActorPerceptionUpdateInfo& UpdateInfo)
{
	// Weird stuff that i cannot fix: Shooting sounds that server makes are all considered as Enemy for everyone, but sight works fine, so we need to skip this if shot was made by a friendly pawn
	// TODO Fix that issue

	if (CurrentMatchState != EMatchState::Gameplay) return; // We dont care about any new data if not in a main game state

	if (UpdateInfo.Stimulus.Type == SenseConfig_Sight->GetSenseID())
	{
		// TODO Set/Reset VisibleEnemy
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
		//UE_LOG(LogTemp, Warning, TEXT("SIGHT"));
	}
	else if (UpdateInfo.Stimulus.Type == SenseConfig_Hearing->GetSenseID())
	{
		// Basically now we are skipping every update if target is in the same team, which may not be preffered logic
		// Actually this workaround can be skipped if we want AIs to always face last heard shot(if no actual enemy in sight of course), no matter its from ally or enemy. More realistic?
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
			if(UpdateInfo.Stimulus.ReceiverLocation == CurrentLastHeardShot) Blackboard->SetValueAsVector(KeyName_LastHeardShot, FAISystem::InvalidLocation);
		}

		//UE_LOG(LogTemp, Warning, TEXT("HEARING"));
	}
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