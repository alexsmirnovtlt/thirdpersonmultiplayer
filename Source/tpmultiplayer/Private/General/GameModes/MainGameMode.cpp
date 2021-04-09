// Aleksandr Smirnov 2021


#include "General/GameModes/MainGameMode.h"

#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "General/Controllers/GameplayAIController.h"
#include "General/Controllers/GamePlayerController.h"
#include "General/Pawns/GameplaySpectatorPawn.h"
#include "General/Actors/GameplayPlayerStart.h"
#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/Actors/GameplayFlagArea.h"
#include "General/HUD/GameplayHUD.h"

const FString AMainGameMode::NewPlayerOptionsNameKey(TEXT("CustomName"));

AMainGameMode::AMainGameMode()
{
	GameStateClass = AGameplayGameState::StaticClass();
	PlayerControllerClass = AGamePlayerController::StaticClass();
	HUDClass = AGameplayHUD::StaticClass();
	//DefaultPawnClass == // Must be nullptr
	SpectatorClass = AGameplaySpectatorPawn::StaticClass();
}

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
	Super::Logout(Exiting);

	if (!Exiting->IsLocalPlayerController())
	{
		if (auto ExitingPC = Cast<AGamePlayerController>(Exiting))
			if (ExitingPC->GetTeamType() != ETeamType::Spectator) RemovePlayerFromAMatch(ExitingPC);
	}
}

// BEGIN Match initialization logic

void AMainGameMode::SetupSpawnLocations()
{
	for (TActorIterator<AGameplayFlagArea> It(GetWorld()); It; ++It)
		FlagPlacements.Add(*It); // Finding all flag locations if exist


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
		AThirdPersonCharacter* Character;
		if(SpawnLocation->TeamType == ETeamType::RedTeam)
			Character = World->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_RedTeam, SpawnLocation->GetTransform(), SpawnParams);
		else 
			Character = World->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_BlueTeam, SpawnLocation->GetTransform(), SpawnParams);
		Character->TeamType = SpawnLocation->TeamType;

		auto AIController = World->SpawnActor<AGameplayAIController>(AIControllerClass);
		AIController->GameState = GameplayState;
		AIController->Possess(Character);

		TeamPawns.Add(Character);
		InGameControllers_AI.Add(AIController);

		Character->OnPawnKilledEvent.AddDynamic(this, &AMainGameMode::OnPawnKilled);
	}
}

void AMainGameMode::AddPlayerToAMatch(AGamePlayerController* PlayerController)
{
	if (HumanPlayersCount_BlueTeam < HumanPlayersCount_RedTeam)
	{
		PlayerController->TeamType = ETeamType::BlueTeam;
		HumanPlayersCount_BlueTeam++;
	}
	else
	{
		PlayerController->TeamType = ETeamType::RedTeam;
		HumanPlayersCount_RedTeam++;
	}

	// Finding non player pawn to posess
	AThirdPersonCharacter* LastAvailablePawn = nullptr; // We are trying to possess a pawn that is still alive but in some cases there will be none so we will possess a pawn that died
	AThirdPersonCharacter* ChosenPawn = nullptr;

	for (auto AvailablePawn : TeamPawns)
	{
		if (AvailablePawn->TeamType == PlayerController->GetTeamType() && !AvailablePawn->IsPlayerControlled())
		{
			if (AvailablePawn->IsAlive())
			{
				ChosenPawn = AvailablePawn;
				break;
			} else if(!LastAvailablePawn) LastAvailablePawn = AvailablePawn;
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
	InGameControllers_Human.Add(PlayerController);

	PlayerController->ForceNetUpdate();
	if (PlayerController->IsLocalPlayerController()) PlayerController->OnRep_Pawn();
}

void AMainGameMode::RemovePlayerFromAMatch(AGamePlayerController* PlayerController)
{
	InGameControllers_Human.Remove(PlayerController);

	if (PlayerController->GetTeamType() == ETeamType::RedTeam) HumanPlayersCount_RedTeam--;
	else HumanPlayersCount_BlueTeam--;

	if (auto PlayerPawn = PlayerController->GetPawn<AThirdPersonCharacter>()) // Need to create new AI and assign it to a pawn
	{
		auto AIController = GetWorld()->SpawnActor<AGameplayAIController>(AIControllerClass);
		InGameControllers_AI.Add(AIController);
		AIController->GameState = GameplayState;

		if (PlayerPawn && PlayerPawn->IsAlive()) AIController->Possess(PlayerPawn);
	}

	PlayerController->TeamType = ETeamType::Spectator;
	PlayerController->ForceNetUpdate();

	// Need to specifically call this on a server controled pawn because we still need to hide UI and server itself will never call it on its own
	if (PlayerController->IsLocalPlayerController()) PlayerController->OnRep_Pawn();
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

	// Giving a random pawn a flag
	auto Pawn = GiveFlagToARandomPawn(ETeamType::RedTeam);
	Pawn->OnRep_FlagOwnerChanged();

	// Initializing all flag areas on a map
	for(auto FlagArea : FlagPlacements)
		FlagArea->InitialSetup(this, GameplayState);

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
	CurrentMatchData.FirstTeam_PlayersAlive = GameplayState->CurrentPlayers_RedTeam;
	CurrentMatchData.SecondTeam_PlayersAlive = GameplayState->CurrentPlayers_BlueTeam;
	CurrentMatchData.AreaWasCaptured = false;
	CurrentMatchData.VIPWasKilled = false;

	for (auto FlagActor : FlagPlacements) FlagActor->ResetFlagState();

	// Giving random pawn a flag
	ETeamType TeamWithAFlag = CurrentMatchData.RedTeamHasFlag ? ETeamType::RedTeam : ETeamType::BlueTeam;
	GiveFlagToARandomPawn(TeamWithAFlag);
	
	ResetPawnsForNewRound(); // Teleports pawns back and possesses died ones

	for (auto TPCPawn : TeamPawns)
	{
		TPCPawn->AuthPrepareForNewGameRound(); // setting back health, idle animation and other optional stuff
		TPCPawn->OnRep_FlagOwnerChanged(); // Showing/hiding flag model on all pawns
	}

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

	DetermineTeamThatWonThatRound(CurrentMatchData); // updating CurrentMatchData based on a few winning conditions

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

	int32 RedTeamSpawnIndex = 0;
	int32 BlueTeamSpawnIndex = 0;

	for (auto Character : TeamPawns)
	{
		int32 ChosenIndex;

		if (Character->TeamType == ETeamType::RedTeam)
			ChosenIndex = GetNextSpawnLocationIndex(RedTeamSpawnIndex, ETeamType::RedTeam);
		else
			ChosenIndex = GetNextSpawnLocationIndex(BlueTeamSpawnIndex, ETeamType::BlueTeam);

		if (ChosenIndex > -1)
			Character->SetActorLocationAndRotation(TeamSpawnLocations[ChosenIndex]->GetActorLocation(), TeamSpawnLocations[ChosenIndex]->GetActorRotation(), false, nullptr, ETeleportType::ResetPhysics);
	
		if (Character->TeamType == ETeamType::RedTeam) RedTeamSpawnIndex = ChosenIndex + 1;
		else BlueTeamSpawnIndex = ChosenIndex + 1;
	}

	int32 ArrayIndex = 0;

	for (auto HumanController : InGameControllers_Human)
	{
		if (!HumanController->GetPawn()) // Possessing a pawn from the same team that is currently unpossessed
		{
			ArrayIndex = GetNextUnpossessedPawnIndex(ArrayIndex, HumanController->GetTeamType());
			if (ArrayIndex > -1) { HumanController->Possess(TeamPawns[ArrayIndex]); ArrayIndex++; }
		}
	}

	ArrayIndex = 0;

	// In some cases when player continiosly clicking play and spectate buttons it may lead to situation when there more or less AIs than needed
	// Easiest fix to that is check the number of AIs in game and spawn/remove them. There is a lot of edge cases (players possesses died pawns ony by one, starting spectating while dying, possessing/spectation same dead pawn) that are hard to track.
	if (InGameControllers_AI.Num() != TeamPawns.Num() - InGameControllers_Human.Num())
	{
		int32 AIControllersNeeded = TeamPawns.Num() - InGameControllers_Human.Num() - InGameControllers_AI.Num();
		if (AIControllersNeeded < 0) // need to remove extra
		{
			for (int32 i = 0; i > AIControllersNeeded; --i)
			{
				AGameplayAIController* FoundControllerWithNoPawn = nullptr;
				for (auto AIController : InGameControllers_AI)
				{
					if (!AIController->GetPawn()) { FoundControllerWithNoPawn = AIController; break; }
				}
				if (FoundControllerWithNoPawn)
				{
					InGameControllers_AI.Remove(FoundControllerWithNoPawn);
					FoundControllerWithNoPawn->Destroy();
				}
			}
		}
		else // need to spawn more
		{
			for (int32 i = 0; i < AIControllersNeeded; ++i)
			{
				auto AIController = GetWorld()->SpawnActor<AGameplayAIController>(AIControllerClass);
				AIController->GameState = GameplayState;
				InGameControllers_AI.Add(AIController);
			}
		}

		if(InGameControllers_AI.Num() != TeamPawns.Num() - InGameControllers_Human.Num())
			UE_LOG(LogTemp, Warning, TEXT("AMainGameMode::ResetPawnsForNewRound - Number of AI controllers is not right, Needed:%d, Got:%d"), TeamPawns.Num() - InGameControllers_Human.Num(), InGameControllers_AI.Num());
	}

	for (auto AIController : InGameControllers_AI)
	{
		if (AIController->GetPawn()) continue;
		
		for (int i = ArrayIndex; i < TeamPawns.Num(); ++i)
		{
			if (!TeamPawns[i]->Controller)
			{
				AIController->Possess(TeamPawns[i]);
				ArrayIndex = i + 1;
				break; // Team doesnt matter because AIs will get their team from a pawn
			}
		}
	}
}

AThirdPersonCharacter* AMainGameMode::GiveFlagToARandomPawn(ETeamType TeamWithFlag)
{
	TArray<AThirdPersonCharacter*> SelectedPawns;
	for (auto TPCPawn : TeamPawns)
	{
		TPCPawn->bHasFlag = false;
		if (TPCPawn->TeamType == TeamWithFlag)
			SelectedPawns.Add(TPCPawn);
	}
	int32 ChosenIndex = FMath::RandRange(0, SelectedPawns.Num() - 1);
	SelectedPawns[ChosenIndex]->bHasFlag = true;
	return SelectedPawns[ChosenIndex];
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
	if (StartingIndex < 0) return -1;
	int32 ValueToReturn = -1;

	for (int32 i = StartingIndex; i < InGameControllers_Human.Num(); ++i)
	{
		if (InGameControllers_Human[i]->GetTeamType() == TeamType) return i;
	}

	return ValueToReturn;
}

int32 AMainGameMode::GetNextUnpossessedPawnIndex(int32 StartingIndex, ETeamType TeamType)
{
	if (StartingIndex < 0) return -1;
	int32 ValueToReturn = -1;

	for (int32 i = StartingIndex; i < TeamPawns.Num(); ++i)
	{
		if (!TeamPawns[i]->Controller && TeamPawns[i]->TeamType == TeamType) return i;
	}

	return ValueToReturn;
}

void AMainGameMode::OnPawnKilled(AThirdPersonCharacter* DiedPawn)
{
	if (DiedPawn->IsPlayerControlled())
	{
		if (auto PlayerController = DiedPawn->GetController<AGamePlayerController>())
		{
			PlayerController->UnPossess();
			PlayerController->ChangeState(NAME_Spectating);
			if (PlayerController->IsLocalPlayerController()) PlayerController->OnRep_Pawn();
		}
	}
	else
	{
		if (auto AIController = DiedPawn->GetController<AGameplayAIController>())
			AIController->UnPossess();
	}

	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	if (DiedPawn->TeamType == ETeamType::RedTeam)
		CurrentMatchData.FirstTeam_PlayersAlive--;
	else if (DiedPawn->TeamType == ETeamType::BlueTeam)
		CurrentMatchData.SecondTeam_PlayersAlive--;

	if (DiedPawn->HasFlag() || GameplayState->CurrentMatchData.FirstTeam_PlayersAlive <= 0 || CurrentMatchData.SecondTeam_PlayersAlive <= 0)
	{
		CurrentMatchData.VIPWasKilled = DiedPawn->HasFlag();
		StopCurrentMatchTimer();
		MatchPhaseStart_RoundEnd();
	}
}

void AMainGameMode::DetermineTeamThatWonThatRound(FMatchData& CurrentMatchData)
{
	// Team won if no other players in other team left
	// TODO Red team is considered FirstTeam, its weird naming and it should be changed

	bool RedTeamWon = true;

	if(CurrentMatchData.VIPWasKilled)
		RedTeamWon = !CurrentMatchData.RedTeamHasFlag; // team that got their vip killed is lost
	else if(CurrentMatchData.AreaWasCaptured)
		RedTeamWon = CurrentMatchData.RedTeamHasFlag; // team that captured area with vip is won
	else if (CurrentMatchData.FirstTeam_PlayersAlive <= 0 || CurrentMatchData.SecondTeam_PlayersAlive <= 0)
		RedTeamWon = CurrentMatchData.FirstTeam_PlayersAlive > 0; // The only remaining team is won
	else
	{ 
		// Round time has ended and no winning conditions happened. Team without vip won 
		RedTeamWon = !CurrentMatchData.RedTeamHasFlag;
	}

	if (RedTeamWon)
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::RedTeamWonLastRound;
		CurrentMatchData.FirstTeam_MatchesWon++;
	}
	else
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::BlueTeamWonLastRound;
		CurrentMatchData.SecondTeam_MatchesWon++;
	}
}

void AMainGameMode::OnAreaStateChanged(EAreaState AreaState)
{
	auto& MatchParameters = GameplayState->GetMatchParameters();
	auto& CurrentMatchData = GameplayState->CurrentMatchData;

	if (CurrentMatchData.MatchState != EMatchState::Gameplay) { UE_LOG(LogTemp, Warning, TEXT("AMainGameMode::OnAreaStateChanged was called when not in Gameplay state!")); return; }

	if (AreaState == EAreaState::Default)
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::Nothing;
	}
	else if (AreaState == EAreaState::BeingCaptured)
	{
		CurrentMatchData.SpecialMessage = EInGameSpecialMessage::AreaCaptureInProgress;
	}
	else if (AreaState == EAreaState::Captured)
	{
		CurrentMatchData.AreaWasCaptured = true;
		StopCurrentMatchTimer();
		MatchPhaseStart_RoundEnd();
	}

	GameplayState->ForceNetUpdate();
	GameplayState->OnRep_MatchStateChanged();
}

// END Match related logic

// DEBUG

void AMainGameMode::Debug_KillRandomPawn()
{
	auto& CurrentMatchData = GameplayState->CurrentMatchData;
	if (CurrentMatchData.MatchState != EMatchState::Gameplay) return;

	TArray<AThirdPersonCharacter*> CharsArray;

	for (auto Pawn : TeamPawns)
		if (Pawn->IsAlive())
			CharsArray.Add(Pawn);

	int32 ChosenIndex = FMath::RandRange(0, CharsArray.Num() - 1);
	
	FDamageEvent DamageEvent;
	CharsArray[ChosenIndex]->TakeDamage(CharsArray[ChosenIndex]->StartingHealth, DamageEvent, nullptr, nullptr);
}

// DEBUG