// Aleksandr Smirnov 2021


#include "General/States/GameplayGameState.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "EngineUtils.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/Actors/GameplayPlayerStart.h"
#include "General/Controllers/GamePlayerController.h"

AGameplayGameState::AGameplayGameState()
{
	MatchParameters = FMatchParameters();
}

void AGameplayGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetWorld())
	{
		if (!GameplayPawnClass_RedTeam || !GameplayPawnClass_BlueTeam) { ensure(GameplayPawnClass_RedTeam && GameplayPawnClass_BlueTeam); return; };

		SetupSpawnLocations();
		SetupPlayableCharacters();
		
		InitialMatchStateSetup();
	}
}

void AGameplayGameState::AddPlayerToAMatch(class APlayerController* PlayerController)
{

}

void AGameplayGameState::RemovePlayerFromAMatch(class APlayerController* PlayerController)
{
	PlayerController->UnPossess();
}

void AGameplayGameState::SetupSpawnLocations()
{
	if (!GetWorld()) { ensure(GetWorld()); return; }

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
	SpectatorInitialSpawnLocation = SpectatorSpawn->GetActorLocation();
	SpectatorInitialSpawnRotation = SpectatorSpawn->GetActorRotation();

	CurrentPlayers_RedTeam = TeamSpawns_Red.Num();
	CurrentPlayers_BlueTeam = TeamSpawns_Blue.Num();
}

void AGameplayGameState::SetupPlayableCharacters()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (auto& item : TeamSpawns_Red)
		GetWorld()->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_RedTeam, item->GetTransform(), SpawnParams);
	for (auto& item : TeamSpawns_Blue)
		GetWorld()->SpawnActor<AThirdPersonCharacter>(GameplayPawnClass_RedTeam, item->GetTransform(), SpawnParams);
}

// BEGIN Match related logic

void AGameplayGameState::OnMatchStateChanged()
{
	// TODO Remove debug info
	if (HasAuthority()) { UE_LOG(LogTemp, Warning, TEXT("SERVER AGameplayGameState::OnMatchStateChanged: %s"), *CurrentMatchData.DebugToString()); }
	else { UE_LOG(LogTemp, Warning, TEXT("CLIENT AGameplayGameState::OnMatchStateChanged: %s"), *CurrentMatchData.DebugToString()); }
	//

	OnMatchDataChangedEvent.Broadcast();
}

void AGameplayGameState::InitialMatchStateSetup()
{
	CurrentMatchData = FMatchData(CurrentPlayers_RedTeam, CurrentPlayers_BlueTeam, MatchParameters.MaxGameRounds, GetServerWorldTimeSeconds());
	
	// DEBUG
	if (HasAuthority())OnMatchStateChanged();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AGameplayGameState::OnMatchTimerEnded, MatchParameters.WarmupPeriodSec, false);
	//
}

void AGameplayGameState::ProceedToNextMatchState()
{

}

void AGameplayGameState::OnMatchTimerEnded()
{
	// DEBUG
	int32 NextMatchState = (int32)CurrentMatchData.MatchState + 1;
	if (NextMatchState > 2) NextMatchState = 0;

	CurrentMatchData.MatchState = (EMatchState) NextMatchState;
	
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

	CurrentMatchData.MatchStartServerTime = GetServerWorldTimeSeconds();

	//
	if(HasAuthority())OnMatchStateChanged();

	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AGameplayGameState::OnMatchTimerEnded, TimerTime, false);

	//
}

// END Match related logic

void AGameplayGameState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameplayGameState, CurrentMatchData);

	DOREPLIFETIME_CONDITION(AGameplayGameState, SpectatorInitialSpawnLocation, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AGameplayGameState, SpectatorInitialSpawnRotation, COND_InitialOnly);

	DOREPLIFETIME_CONDITION(AGameplayGameState, CurrentPlayers_RedTeam, COND_InitialOnly); 
	DOREPLIFETIME_CONDITION(AGameplayGameState, CurrentPlayers_BlueTeam, COND_InitialOnly);
}