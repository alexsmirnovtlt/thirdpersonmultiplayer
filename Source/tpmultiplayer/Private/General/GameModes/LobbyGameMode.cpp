// Aleksandr Smirnov 2021


#include "General/GameModes/LobbyGameMode.h"

#include "OnlineSessionSettings.h"
#include "Widgets/SWeakWidget.h"
#include "OnlineSubsystem.h"

#include "Slate/LobbyFoundGameInfoWidgetStyle.h"
#include "Slate/LobbyMenuSlateWidgetStyle.h"
#include "General/MultiplayerGameInstance.h"
#include "Slate/SLobbyWidget.h"

FName ALobbyGameMode::CreatedSessionName(TEXT(""));
const FName ALobbyGameMode::SERVER_NAME_SETTINGS_KEY("ServerName");

// TODO delegate handle
// TODO less button enable calls
// Lobby widget reference to session items

void ALobbyGameMode::StartPlay()
{
	Super::StartPlay();

	CreateMainWidget();
	InitOnlineSubsystem();

	// Showing mouse cursor when fullscreen
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData = FInputModeUIOnly();
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
	}
}

AActor* ALobbyGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	return Player; // Instead of looking for PlayerStart actors just get Controller`s location (should be FVector::ZeroVector)
}

void ALobbyGameMode::InitOnlineSubsystem()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem) return;

	SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &ALobbyGameMode::OnCreateSessionComplete);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &ALobbyGameMode::OnFindSessionsComplete);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &ALobbyGameMode::OnJoinSessionComplete);

	// Closing previous session if exists (happens when hosting player returned from gameplay level or reopened the application)
	if (!CreatedSessionName.IsNone())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(CreatedSessionName);
		if (ExistingSession != nullptr)
		{
			LobbyWidget.Get()->SetButtonEnabled_Host(false);
			LobbyWidget.Get()->SetButtonEnabled_Search(false);
			LobbyWidget.Get()->SetButtonEnabled_Join(false);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &ALobbyGameMode::OnDestroySessionComplete);
			SessionInterface->DestroySession(CreatedSessionName);
			return;
		}
	}
}

void ALobbyGameMode::CreateMainWidget()
{
	// Some checks before Slate Widget creation 
	if (!GEngine || (GEngine && !GEngine->GameViewport)) return;
	if (!MenuStyleClass || !SessionItemStyleClass) { UE_LOG(LogTemp, Error, TEXT("ALobbyGameMode: Default slate classes must be assigned!")); return; };

	// Getting widget style info that we specified in blueprint. Will be used in SLobbyWidget::Construct()
	auto MainStyle = MenuStyleClass.GetDefaultObject();
	auto ItemStyle = SessionItemStyleClass.GetDefaultObject();

	// Creating Lobby Slate Widget and passing parameters to it
	LobbyWidget = SNew(SLobbyWidget).LobbyGameMode(this).LobbyStyle(MainStyle).SessionItemStyle(ItemStyle);

	// Placing SLobbyWidget in a WeakPtr container so when this GameMode gets destroyed, TSharedPtr LobbyWidget will decrement one reference and LobbyWidget will be destroyed
	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(LobbyWidgetContainer, SWeakWidget)
		.PossiblyNullContent(LobbyWidget)
	);
}

void ALobbyGameMode::OnFindSessionsComplete(bool Success)
{
	auto SLobbyWidget = LobbyWidget.Get();

	SLobbyWidget->SetButtonEnabled_Host(true);
	SLobbyWidget->SetButtonEnabled_Search(true);

	SLobbyWidget->ClearSessionsList();

	if (!Success || !SessionSearchResults.IsValid())
	{
		SLobbyWidget->DisplayNoSessionsFound();
		return;
	}

	auto SearchResults = SessionSearchResults.Get()->SearchResults;
	if (SearchResults.Num() == 0)
	{
		SLobbyWidget->DisplayNoSessionsFound();
		return;
	}

	int32 sessionsShown = 0;

	for (int32 index = 0; index < SearchResults.Num(); ++index)
	{
		auto& resultItem = SearchResults[index];

		int32 maxPlayers = resultItem.Session.SessionSettings.NumPublicConnections;
		int32 currentPlayers = maxPlayers - resultItem.Session.NumOpenPublicConnections;
		if (maxPlayers == currentPlayers) continue;

		FString SessioNameStr = "";
		resultItem.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, SessioNameStr);
		if (SessioNameStr.IsEmpty()) continue;

		SLobbyWidget->AddFoundSession(SessioNameStr, currentPlayers, maxPlayers, index);
	}

	if (sessionsShown == 0) SLobbyWidget->DisplayNoSessionsFound();
	
	SLobbyWidget->ResetButtonState();
}

void ALobbyGameMode::OnDestroySessionComplete(FName SessionName, bool Success)
{
	UE_LOG(LogTemp, Warning, TEXT("Session \"%s\" was destroyed"), *SessionName.ToString());
	LobbyWidget.Get()->ResetButtonState();
}

void ALobbyGameMode::OnStartHosting(FText& SessionName, FText& PlayerName)
{
	if (!SessionInterface.IsValid() || !GetWorld()) return;

	CreatedSessionName = FName(*SessionName.ToString());

	auto GameInstance = CastChecked<UMultiplayerGameInstance>(GetWorld()->GetGameInstance());

	// TODO delegate

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true; // For Null Online Subsystem to work, should be changed in a real application 
	SessionSettings.NumPublicConnections = GameInstance->GetMaxOnlinePlayers();
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;

	// Passing our custom parameter to correctly display available session names to browsing players
	SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, SessionName.ToString(), EOnlineDataAdvertisementType::Type::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, CreatedSessionName, SessionSettings);
}

void ALobbyGameMode::OnCreateSessionComplete(FName SessionName, bool Success)
{
	auto World = GetWorld();
	if (!World || !Success)
	{
		LobbyWidget.Get()->ResetButtonState();
		return;
	}

	auto GameInstance = CastChecked<UMultiplayerGameInstance>(GetWorld()->GetGameInstance());

	World->ServerTravel(GameInstance->GetGameplayMapNameForHost(), true);
}

void ALobbyGameMode::OnStartSearchingGames()
{
	if (!SessionInterface.IsValid()) return;

	if (!SessionSearchResults.IsValid())
	{
		SessionSearchResults = MakeShareable(new FOnlineSessionSearch());
		SessionSearchResults->MaxSearchResults = MaxSearchResults;
		SessionSearchResults->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	}

	SessionInterface->FindSessions(0, SessionSearchResults.ToSharedRef());
}

void ALobbyGameMode::OnStartJoining(FText& SessionName, FText& PlayerName, int32 SessionIndex)
{
	if (!SessionInterface.IsValid()) return;

	// TODO delegate

	SessionInterface->JoinSession(
		0,
		FName(*SessionName.ToString()),
		SessionSearchResults->SearchResults[SessionIndex]
	);
}

void ALobbyGameMode::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type SessionType)
{
	if (!SessionInterface.IsValid()) return;

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address)) {
		UE_LOG(LogTemp, Warning, TEXT("Could not get connect string."));
		return;
	}

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}