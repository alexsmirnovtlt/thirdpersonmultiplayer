// Aleksandr Smirnov 2021


#include "General/GameModes/MainGameMode.h"

#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "General/Controllers/GamePlayerController.h"
#include "General/Actors/GameplayPlayerStart.h"
#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayPlayerState.h"

const FString AMainGameMode::NewPlayerOptionsNameKey(TEXT("CustomName"));

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	if (!GetWorld() || !GameplayPawnClass_RedTeam || !GameplayPawnClass_BlueTeam) { ensure(false); return; }

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

		if (FoundActor->TeamType == ETeamType::BlueTeam) TeamSpawns_Blue.Add(FoundActor);
		else if (FoundActor->TeamType == ETeamType::RedTeam) TeamSpawns_Red.Add(FoundActor);
		else SpectatorSpawn = FoundActor;
	}

	if (!SpectatorSpawn || TeamSpawns_Blue.Num() == 0 || TeamSpawns_Red.Num() == 0)
	{
		int32 SpectatorSpawns = SpectatorSpawn ? 1 : 0;
		int32 BlueTeamSpawns = TeamSpawns_Blue.Num();
		int32 RedTeamSpawns = TeamSpawns_Red.Num();

		UE_LOG(LogTemp, Error, TEXT("AMainGameMode::SetupSpawnLocations Not enough Player Starts were found! Spectators: %d, BlueTeam Spawns: %d, RedTeam Spawns: %d"), SpectatorSpawns, BlueTeamSpawns, RedTeamSpawns);

		if (SpectatorSpawns == 0) SpectatorSpawn = GetWorld()->SpawnActor<AActor>();
		if (BlueTeamSpawns == 0) TeamSpawns_Blue.Add(GetWorld()->SpawnActor<AActor>());
		if (RedTeamSpawns == 0) TeamSpawns_Red.Add(GetWorld()->SpawnActor<AActor>());
	}

	// Setting up initial spectator location
	GameplayState->SpectatorInitialSpawnLocation = SpectatorSpawn->GetActorLocation();
	GameplayState->SpectatorInitialSpawnRotation = SpectatorSpawn->GetActorRotation();

	GameplayState->CurrentPlayers_RedTeam = TeamSpawns_Red.Num();
	GameplayState->CurrentPlayers_BlueTeam = TeamSpawns_Blue.Num();
}

void AMainGameMode::SetupPlayableCharacters()
{
FActorSpawnParameters SpawnParams;
SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

for (auto& item : TeamSpawns_Red)
TeamPawns_Red.Add(GetWorld()->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_RedTeam, item->GetTransform(), SpawnParams));
for (auto& item : TeamSpawns_Blue)
TeamPawns_Blue.Add(GetWorld()->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_RedTeam, item->GetTransform(), SpawnParams));

// TODO Assign AI to all pawns
}

void AMainGameMode::AddPlayerToAMatch(AGamePlayerController* PlayerController)
{
	auto PlayerState = PlayerController->GetGamePlayerState();
	if (!PlayerState) { ensure(false); return; };

	if (HumanPlayersCount_RedTeam <= HumanPlayersCount_BlueTeam)
	{
		PlayerController->Possess(TeamPawns_Red[HumanPlayersCount_RedTeam]);
		HumanPlayersCount_RedTeam++;
		PlayerState->TeamType = ETeamType::RedTeam;
	}
	else
	{
		PlayerController->Possess(TeamPawns_Blue[HumanPlayersCount_BlueTeam]);
		HumanPlayersCount_BlueTeam++;
		PlayerState->TeamType = ETeamType::BlueTeam;
	}

	if (PlayerController->IsLocalController()) PlayerController->OnRep_Pawn();
}

void AMainGameMode::RemovePlayerFromAMatch(AGamePlayerController* PlayerController)
{
	auto PlayerState = PlayerController->GetGamePlayerState();
	if (!PlayerState) { ensure(false); return; };

	auto PlayerPawn = PlayerController->GetPawn();

	if (PlayerState->TeamType == ETeamType::RedTeam) HumanPlayersCount_RedTeam--;
	else HumanPlayersCount_BlueTeam--;

	PlayerState->TeamType = ETeamType::Spectator;
	PlayerController->UnPossess();

	if (PlayerController->IsLocalController()) PlayerController->OnRep_Pawn();

	// TODO Assign AI to a freed pawn
}

// END Match initialization logic

// BEGIN Match related logic

void AMainGameMode::InitialMatchStateSetup()
{
	GameplayState->CurrentMatchData = FMatchData(
		TeamSpawns_Red.Num(),
		TeamSpawns_Blue.Num(),
		GameplayState->GetMatchParameters().MaxGameRounds,
		GameplayState->GetServerWorldTimeSeconds()
	);
	GameplayState->CurrentMatchData.MatchState = EMatchState::RoundEnd; // will be moving to next stage (first) right after that

	ProceedToNextMatchState();
}

void AMainGameMode::ProceedToNextMatchState()
{
	auto& MatchParameters = GameplayState->GetMatchParameters();
	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	int32 NextMatchState = (int32)CurrentMatchData.MatchState + 1;
	if (NextMatchState > 2) NextMatchState = 0; // hardcode, we have 3 EMatchState values

	CurrentMatchData.MatchState = (EMatchState)NextMatchState;

	// Deciding on timer time
	float TimerTime;
	if (CurrentMatchData.MatchState == EMatchState::Warmup)
	{
		TimerTime = MatchParameters.WarmupPeriodSec;
	}
	else if (CurrentMatchData.MatchState == EMatchState::Gameplay)
	{
		TimerTime = MatchParameters.MatchPeriodSec;
	}
	else if (CurrentMatchData.MatchState == EMatchState::RoundEnd)
	{
		TimerTime = MatchParameters.EndRoundPeriodSec;
	}

	// Main state change logic
	if (CurrentMatchData.MatchState == EMatchState::Warmup)
	{
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
	}
	else if (CurrentMatchData.MatchState == EMatchState::Gameplay)
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::Nothing;
	}
	else if (CurrentMatchData.MatchState == EMatchState::RoundEnd)
	{
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
	}

	CurrentMatchData.RedTeamHasFlag = !CurrentMatchData.RedTeamHasFlag;
	CurrentMatchData.MatchStartServerTime = GameplayState->GetServerWorldTimeSeconds();

	// Finalize
	GameplayState->ForceNetUpdate(); // TODO make it so GameplayState will not check for replication updates automatically and we update it manually like that. NetUpdateFrequency 0 in GameplayState may not be it
	GameplayState->OnRep_MatchStateChanged();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::ProceedToNextMatchState, TimerTime, false);
}

void AMainGameMode::OnMatchTimerEnded()
{
	/*auto& MatchParameters = GameplayState->GetMatchParameters();
	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	// START DEBUG
	int32 NextMatchState = (int32)CurrentMatchData.MatchState + 1;
	if (NextMatchState > 2) NextMatchState = 0;

	CurrentMatchData.MatchState = (EMatchState)NextMatchState;

	float TimerTime;

	if (CurrentMatchData.MatchState == EMatchState::Warmup)
	{
		TimerTime = MatchParameters.WarmupPeriodSec;
	}
	else if (CurrentMatchData.MatchState == EMatchState::Gameplay)
	{
		TimerTime = MatchParameters.MatchPeriodSec;
	}
	else if (CurrentMatchData.MatchState == EMatchState::RoundEnd)
	{
		TimerTime = MatchParameters.EndRoundPeriodSec;
	}

	CurrentMatchData.MatchStartServerTime = GameplayState->GetServerWorldTimeSeconds();

	//
	GameplayState->OnMatchStateChanged();

	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::OnMatchTimerEnded, TimerTime, false);
	*/
	// END DEBUG
}

// END Match related logic