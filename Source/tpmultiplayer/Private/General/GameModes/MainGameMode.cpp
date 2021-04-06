// Aleksandr Smirnov 2021


#include "General/GameModes/MainGameMode.h"

#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "General/Controllers/GameplayAIController.h"
#include "General/Controllers/GamePlayerController.h"
#include "General/Actors/GameplayPlayerStart.h"
#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayPlayerState.h"

const FString AMainGameMode::NewPlayerOptionsNameKey(TEXT("CustomName"));

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	if (!GetWorld() || !GameplayPawnClass_RedTeam || !GameplayPawnClass_BlueTeam || !AIControllerClass) { ensure(false); return; }

	GameplayState = GetGameState<AGameplayGameState>();
	if (!GameplayState) { ensure(false); return; }
	
	SetupSpawnLocations(); // Get actors from level that will be used as spawn points and flag locations
	SetupPlayableCharacters(); // Spawn all players and possess them with AIs

	InitialMatchStateSetup(); // Start main game Loop(csgo like): 1. warmup where players teleported to spawn locations and cannot move, 2. main phase where players shoot each other and defend/attack the flag 3. end match phase that starts the loop again
}

APlayerController* AMainGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// Creating player controler as usual. But its name will be set to &Name parameter from Options. Usually its fine because f.e Steam will return Steam Name (NULL subsystem returns some PC UUID or something)
	// In out case player specifies his name in a Lobby before joining the game, so CustomName option parameter will be used.

	if (Options.Contains(NewPlayerOptionsNameKey))
	{
		// Insted of parsing and finding CustomName value we just change option keys
		FString NewOptions = Options.Replace(TEXT("?Name="), TEXT("?unusedparam="), ESearchCase::CaseSensitive);
		NewOptions = Options.Replace(*NewPlayerOptionsNameKey, TEXT("Name"), ESearchCase::CaseSensitive);
		return Super::Login(NewPlayer, InRemoteRole, Portal, NewOptions, UniqueId, ErrorMessage);
	}
	else return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void AMainGameMode::Logout(AController* Exiting)
{
	if (auto ExitingPC = Cast<AGamePlayerController>(Exiting))
	{
		if(ExitingPC->IsInState(NAME_Playing)) RemovePlayerFromAMatch(ExitingPC);
	}

	Super::Logout(Exiting);
}

// BEGIN Match initialization logic

void AMainGameMode::SetupSpawnLocations()
{
	for (TActorIterator<AGameplayPlayerStart> It(GetWorld()); It; ++It)
	{
		AGameplayPlayerStart* FoundActor = *It;

		if(FoundActor->TeamType == ETeamType::Spectator) SpectatorSpawn = FoundActor;
		else
		{
			if (FoundActor->TeamType == ETeamType::RedTeam) GameplayState->CurrentPlayers_RedTeam++;
			else GameplayState->CurrentPlayers_BlueTeam++;
			TeamSpawnLocations.Add(FoundActor);
		}
	}

	if (!SpectatorSpawn || GameplayState->CurrentPlayers_RedTeam == 0 || GameplayState->CurrentPlayers_BlueTeam == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("AMainGameMode::SetupSpawnLocations Not enough Player Starts were found!"));

		if (!SpectatorSpawn) SpectatorSpawn = GetWorld()->SpawnActor<AActor>();
		if (GameplayState->CurrentPlayers_BlueTeam == 0) { auto NewActor = GetWorld()->SpawnActor<AGameplayPlayerStart>(); NewActor->TeamType = ETeamType::BlueTeam; TeamSpawnLocations.Add(NewActor);  GameplayState->CurrentPlayers_BlueTeam++; }
		if (GameplayState->CurrentPlayers_RedTeam == 0) { auto NewActor = GetWorld()->SpawnActor<AGameplayPlayerStart>(); NewActor->TeamType = ETeamType::RedTeam; TeamSpawnLocations.Add(NewActor);  GameplayState->CurrentPlayers_RedTeam++; }
	}

	// Setting up initial spectator location
	GameplayState->SpectatorInitialSpawnLocation = SpectatorSpawn->GetActorLocation();
	GameplayState->SpectatorInitialSpawnRotation = SpectatorSpawn->GetActorRotation();
}

void AMainGameMode::SetupPlayableCharacters()
{
	auto World = GetWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (auto& SpawnLocation : TeamSpawnLocations)
	{
		auto Character = World->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_RedTeam, SpawnLocation->GetTransform(), SpawnParams);
		Character->TeamType = SpawnLocation->TeamType;

		auto AIController = World->SpawnActor<AGameplayAIController>(AIControllerClass);
		AIController->GameState = GameplayState;
		AIController->Possess(Character);

		TeamPawns.Add(Character);
		InGameControllers_AI.Add(AIController);
	}
}

void AMainGameMode::AddPlayerToAMatch(AGamePlayerController* PlayerController)
{
	auto PlayerState = PlayerController->GetGamePlayerState();
	if (!PlayerState) { ensure(false); return; };

	ETeamType TeamToJoin = ETeamType::RedTeam;
	if (HumanPlayersCount_BlueTeam < HumanPlayersCount_RedTeam)
	{
		TeamToJoin = ETeamType::BlueTeam;
		HumanPlayersCount_BlueTeam++;
	}
	else HumanPlayersCount_RedTeam++;

	// Finding non player pawn to posess
	AThirdPersonCharacter* LastAvailablePawn = nullptr; // We are trying to possess a pawn that is still alive but in some cases there will be none so we will possess a pawn that died
	AThirdPersonCharacter* ChosenPawn = nullptr;

	for (auto AvailablePawn : TeamPawns)
	{
		if (AvailablePawn->TeamType == TeamToJoin && !AvailablePawn->IsPlayerControlled())
		{
			LastAvailablePawn = AvailablePawn;
			if (AvailablePawn->IsAlive())
			{
				ChosenPawn = AvailablePawn;
				break;
			}
		}
	}

	if (ChosenPawn == nullptr) // everyone from this team is dead, possessing died pawn
		ChosenPawn = LastAvailablePawn;

	if (auto PossessedAI = Cast<AGameplayAIController>(ChosenPawn->GetController()))
	{
		PossessedAI->UnPossess();
		InGameControllers_AI.Remove(PossessedAI);
		PossessedAI->Destroy();
	}

	PlayerController->Possess(ChosenPawn);
	PlayerController->GetGamePlayerState()->TeamType = TeamToJoin;
	
	if (PlayerController->IsLocalController()) PlayerController->OnRep_Pawn();
}

void AMainGameMode::RemovePlayerFromAMatch(AGamePlayerController* PlayerController)
{
	auto PlayerState = PlayerController->GetGamePlayerState();
	if (!PlayerState) { ensure(false); return; };

	if (PlayerState->TeamType == ETeamType::Spectator) { InGameControllers_Human.Remove(PlayerController); return; } // TODO may be redundant

	auto PlayerPawn = PlayerController->GetPawn();

	if (PlayerState->TeamType == ETeamType::RedTeam) HumanPlayersCount_RedTeam--;
	else HumanPlayersCount_BlueTeam--;

	PlayerState->TeamType = ETeamType::Spectator;
	PlayerController->UnPossess();

	if (PlayerController->IsLocalController()) PlayerController->OnRep_Pawn();

	// Newly created AI controller gets assigned to a pawn
	auto AIController = GetWorld()->SpawnActor<AGameplayAIController>(AIControllerClass);
	AIController->GameState = GameplayState;
	AIController->Possess(PlayerPawn);

	InGameControllers_AI.Add(AIController);
}

// END Match initialization logic

// BEGIN Match related logic

void AMainGameMode::InitialMatchStateSetup()
{
	GameplayState->CurrentMatchData = FMatchData(
		GameplayState->CurrentPlayers_RedTeam,
		GameplayState->CurrentPlayers_BlueTeam,
		GameplayState->GetMatchParameters().MaxGameRounds,
		GameplayState->GetServerWorldTimeSeconds()
	);

	GameplayState->OnRep_MatchStateChanged();

	GameplayState->ForceNetUpdate();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::MatchPhaseStart_Gameplay, GameplayState->GetMatchParameters().WarmupPeriodSec, false);
}

void AMainGameMode::MatchPhaseStart_Warmup()
{
	auto& MatchParameters = GameplayState->GetMatchParameters();
	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	MatchPhaseEnd_RoundEnd(MatchParameters, CurrentMatchData);

	if (CurrentMatchData.FirstTeam_MatchesWon >= MatchParameters.MaxGameRoundsToWin)
	{
		CurrentMatchData.FirstTeam_MatchesWon = 0;
		CurrentMatchData.SecondTeam_MatchesWon = 0;
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::RedTeamWonLastGame;
		CurrentMatchData.CurrentRound = 1;
	}
	else if (CurrentMatchData.SecondTeam_MatchesWon >= MatchParameters.MaxGameRoundsToWin)
	{
		CurrentMatchData.FirstTeam_MatchesWon = 0;
		CurrentMatchData.SecondTeam_MatchesWon = 0;
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::BlueTeamWonLastGame;
		CurrentMatchData.CurrentRound = 1;
	}
	else
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::Nothing;
		CurrentMatchData.CurrentRound += 1;
	}

	CurrentMatchData.MatchState = EMatchState::Warmup;
	CurrentMatchData.RedTeamHasFlag = !CurrentMatchData.RedTeamHasFlag;
	CurrentMatchData.MatchStartServerTime = GameplayState->GetServerWorldTimeSeconds();

	ResetPawnsForNewRound();

	// Finalize
	GameplayState->ForceNetUpdate(); // TODO make it so GameplayState will not check for replication updates automatically and we update it manually like that. NetUpdateFrequency 0 in GameplayState may not be it
	GameplayState->OnRep_MatchStateChanged();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::MatchPhaseStart_Gameplay, MatchParameters.WarmupPeriodSec, false);
}

void AMainGameMode::MatchPhaseStart_Gameplay()
{
	auto& MatchParameters = GameplayState->GetMatchParameters();
	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	MatchPhaseEnd_Warmup(MatchParameters, CurrentMatchData);

	CurrentMatchData.MatchState = EMatchState::Gameplay;
	CurrentMatchData.SpecialMessage = EInGameSpecialMessage::Nothing;
	CurrentMatchData.MatchStartServerTime = GameplayState->GetServerWorldTimeSeconds();
	
	// Finalize
	GameplayState->ForceNetUpdate();
	GameplayState->OnRep_MatchStateChanged();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::MatchPhaseStart_RoundEnd, MatchParameters.MatchPeriodSec, false);
}

void AMainGameMode::MatchPhaseStart_RoundEnd()
{
	auto& MatchParameters = GameplayState->GetMatchParameters();
	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	MatchPhaseEnd_Gameplay(MatchParameters, CurrentMatchData);

	if (CurrentMatchData.RedTeamHasFlag)
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::BlueTeamWonLastRound;
		CurrentMatchData.SecondTeam_MatchesWon++;
	}
	else
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::RedTeamWonLastRound;
		CurrentMatchData.FirstTeam_MatchesWon++;
	}

	CurrentMatchData.MatchState = EMatchState::RoundEnd;
	CurrentMatchData.MatchStartServerTime = GameplayState->GetServerWorldTimeSeconds();

	// Finalize
	GameplayState->ForceNetUpdate();
	GameplayState->OnRep_MatchStateChanged();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::MatchPhaseStart_Warmup, MatchParameters.EndRoundPeriodSec, false);
}

void AMainGameMode::MatchPhaseEnd_Warmup(const FMatchParameters& MatchParameters, const FMatchData& CurrentMatchData)
{
	// Granting moving, aiming, shooting abilities to everyone

	// TODO actually do that
}

void AMainGameMode::MatchPhaseEnd_Gameplay(const FMatchParameters& MatchParameters, const FMatchData& CurrentMatchData)
{
	// Revoke aiming and shooting abilities from everyone

	// TODO actually do that
}

void AMainGameMode::MatchPhaseEnd_RoundEnd(const FMatchParameters& MatchParameters, const FMatchData& CurrentMatchData)
{
	// Removing ability to move from everyone
	// Granting one pawn an ability to place a flag, revoking that ability from others
	// Removing ability to pick up a flag from one team, grantimg it to another team

	// TODO actually do that
}

void AMainGameMode::StopCurrentMatchTimer()
{
	auto& TimerManager = GetWorld()->GetTimerManager();
	if (TimerManager.IsTimerActive(MatchTimerHandle))
		TimerManager.ClearTimer(MatchTimerHandle);
}

void AMainGameMode::ResetPawnsForNewRound()
{
	// Happens when new game round begins. All pawns restart and teleport to starting locations. All connected players and AI controllers possess a pawn if wasnt possessing one before.

	for (auto Character : TeamPawns)
	{
		int32 RedTeamSpawnIndex = 0;
		int32 BlueTeamSpawnIndex = 0;

		int32 ChosenIndex = -1;

		if (Character->TeamType == ETeamType::RedTeam)
		{
			RedTeamSpawnIndex = GetNextSpawnLocationIndex(RedTeamSpawnIndex, ETeamType::RedTeam);
			ChosenIndex = RedTeamSpawnIndex;
		}
		else
		{
			BlueTeamSpawnIndex = GetNextSpawnLocationIndex(BlueTeamSpawnIndex, ETeamType::BlueTeam);
			ChosenIndex = BlueTeamSpawnIndex;
		}

		Character->SetActorLocationAndRotation(TeamSpawnLocations[ChosenIndex]->GetActorLocation(), TeamSpawnLocations[ChosenIndex]->GetActorRotation());
	}

	for (auto HumanController : InGameControllers_Human)
	{
		// TODO possess if not already
	}

	for (auto AIController : InGameControllers_Human)
	{
		// TODO possess if not already
	}
}

int32 AMainGameMode::GetNextSpawnLocationIndex(int32 StartingIndex, ETeamType TeamType)
{
	int32 ValueToReturn = -1;
	for (int32 i = StartingIndex; i < TeamSpawnLocations.Num(); ++i)
	{
		if (TeamSpawnLocations[i]->TeamType == TeamType) return i;
	}

	return ValueToReturn;
}

int32 AMainGameMode::GetNextPlayerControllerIndex(int32 StartingIndex, ETeamType TeamType)
{
	int32 ValueToReturn = -1;
	for (int32 i = StartingIndex; i < InGameControllers_Human.Num(); ++i)
	{
		if (InGameControllers_Human[i]->GetGamePlayerState()->TeamType == TeamType) return i;
	}

	return ValueToReturn;
}

// END Match related logic