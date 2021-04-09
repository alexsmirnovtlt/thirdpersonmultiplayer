// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameplayHUD.generated.h"

/**
 * 
 */
UCLASS(abstract)
class TPMULTIPLAYER_API AGameplayHUD : public AHUD
{
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void MainMenu_Show();
	void MainMenu_Hide();
	void MainMenu_Toggle();

	void GameplayMenu_Show();
	void GameplayMenu_Hide();
	void GameplayMenu_Toggle();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Slate")
	TSubclassOf <class UGameplayMainMenuWidgetStyle> MainMenuStyleClass;
	UPROPERTY(EditDefaultsOnly, Category = "Slate")
	TSubclassOf <class UGameplayMainHUDWidgetStyle> GameplayHUDStyleClass;

	TSharedPtr<class SGameplayMainMenuWidget> MainMenuWidget;
	TSharedPtr<class SGameplayMainHUDWidget> GameplayWidget;
	TSharedPtr<class SWeakWidget> MainMenuWidgetContainer;
	TSharedPtr<class SWeakWidget> GemaplayHUDWidgetContainer;

	class AGamePlayerController* GameplayPlayerController;

	UFUNCTION()
	void OnMatchDataUpdated();
};
