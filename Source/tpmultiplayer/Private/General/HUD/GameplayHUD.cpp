// Aleksandr Smirnov 2021


#include "General/HUD/GameplayHUD.h"

#include "Widgets/SWeakWidget.h"

#include "General/Controllers/GamePlayerController.h"
#include "Slate/GameplayMainMenuWidgetStyle.h"
#include "Slate/GameplayMainHUDWidgetStyle.h"
#include "General/States/GameplayGameState.h"
#include "Slate/SGameplayMainHUDWidget.h"
#include "Slate/GameplayMainMenu.h"

void AGameplayHUD::BeginPlay()
{
	Super::BeginPlay();
	
	GameplayPlayerController = Cast<AGamePlayerController>(GetOwningPlayerController());
	if (!GameplayPlayerController) UE_LOG(LogTemp, Error, TEXT("AGameplayHUD::BeginPlay() was not able to cast GameplayPlayerController!"));
	
	// Event that will fire every time MatchData was received from the server so we need to update out HUD info about current match state (time, score, etc)
	GameplayPlayerController->GetGameplayState()->OnMatchDataChangedEvent.AddDynamic(this, &AGameplayHUD::OnMatchDataUpdated);

	MainMenu_Show(); // Creating and showing main menu widget so player can join a game or return to lobby
}

void AGameplayHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MainMenu_Hide();
	GameplayMenu_Hide();

	GameplayPlayerController->GetGameplayState()->OnMatchDataChangedEvent.RemoveDynamic(this, &AGameplayHUD::OnMatchDataUpdated);

	Super::EndPlay(EndPlayReason);
}

void AGameplayHUD::MainMenu_Show()
{
	if (!GEngine || (GEngine && !GEngine->GameViewport)) return;
	if (!MainMenuStyleClass) { UE_LOG(LogTemp, Error, TEXT("AGameplayHUD: Defaults must be assigned!")); return; };

	if (!MainMenuWidget.IsValid())
		MainMenuWidget = SNew(SGameplayMainMenuWidget).PlayerController(GameplayPlayerController).MainMenuStyle(MainMenuStyleClass.GetDefaultObject());

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(WidgetContainer, SWeakWidget)
		.PossiblyNullContent(MainMenuWidget)
	);

	GameplayPlayerController->ChangeInputMode(true);
}

void AGameplayHUD::MainMenu_Hide()
{
	if (!MainMenuWidget.IsValid()) return;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(WidgetContainer.ToSharedRef());

		//WidgetContainer.Reset();
		MainMenuWidget.Reset();
	}

	GameplayPlayerController->ChangeInputMode(false);
}

void AGameplayHUD::MainMenu_Toggle()
{
	if (MainMenuWidget.IsValid()) MainMenu_Hide();
	else MainMenu_Show();
}

void AGameplayHUD::GameplayMenu_Toggle()
{
	if (GameplayWidget.IsValid()) GameplayMenu_Hide();
	else GameplayMenu_Show();
}

void AGameplayHUD::GameplayMenu_Show()
{
	if (!GEngine || (GEngine && !GEngine->GameViewport)) return;
	if (!GameplayHUDStyleClass) { UE_LOG(LogTemp, Error, TEXT("AGameplayHUD: Defaults must be assigned!")); return; };

	if (!GameplayWidget.IsValid())
		GameplayWidget = SNew(SGameplayMainHUDWidget).MainStyle(GameplayHUDStyleClass.GetDefaultObject());

	OnMatchDataUpdated(); // updating widget data manually on widget creation

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(WidgetContainer, SWeakWidget)
		.PossiblyNullContent(GameplayWidget)
	);
}

void AGameplayHUD::GameplayMenu_Hide()
{
	if (!GameplayWidget.IsValid()) return;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(WidgetContainer.ToSharedRef());

		GameplayWidget.Reset();
	}
}

void AGameplayHUD::OnMatchDataUpdated()
{
	// Event from GameState that it received updated MatchData struct so we can update HUD widget with that info 
	if (!GameplayWidget.IsValid() || !IsValid(GameplayPlayerController)) return;
	auto GameState = GameplayPlayerController->GetGameplayState();
	if (!GameState) return;

	auto& MatchData = GameState->GetCurrentMatchData();
	auto& MatchParameters = GameState->GetMatchParameters();
	float TimePassed = GameState->GetServerWorldTimeSeconds() - MatchData.MatchStartServerTime;

	GameplayWidget.Get()->UpdateWidgetData(MatchData, MatchParameters, TimePassed);
}