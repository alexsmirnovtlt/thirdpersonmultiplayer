// Aleksandr Smirnov 2021


#include "General/GameModes/MainGameMode.h"

#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

#include "General/Actors/GameplayPlayerStart.h"
#include "General/Pawns/ThirdPersonCharacter.h"

const FString AMainGameMode::NewPlayerOptionsNameKey(TEXT("CustomName"));

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	if (!GetWorld() || !GameplayPawnClass_RedTeam || !GameplayPawnClass_BlueTeam) { ensure(false); return; }

	GameplayState = GetGameState<AGameplayGameState>();

	SetupSpawnLocations();
	SetupPlayableCharacters();
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
	if (auto ExitingPC = Cast<APlayerController>(Exiting))
		RemovePlayerFromAMatch(ExitingPC);

	Super::Logout(Exiting);
}

// BEGIN Match related logic

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
}

void AMainGameMode::AddPlayerToAMatch(class APlayerController* PlayerController)
{
	// TODO Possess pawn from a correct team
	
	// DEBUG
	if(!TeamPawns_Blue[0]->IsPlayerControlled()) PlayerController->Possess(TeamPawns_Blue[0]);
	else PlayerController->Possess(TeamPawns_Red[0]);
	
	if(PlayerController->IsLocalController()) PlayerController->OnRep_Pawn();
	//
}

void AMainGameMode::RemovePlayerFromAMatch(class APlayerController* PlayerController)
{
	PlayerController->UnPossess();
}

// END Match related logic