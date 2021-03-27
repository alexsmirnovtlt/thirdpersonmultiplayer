// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Main Widget for gameplay menu
 */
class TPMULTIPLAYER_API SGameplayMainMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameplayMainMenuWidget)
	{}
	SLATE_ARGUMENT(TWeakObjectPtr<class AGamePlayerController>, PlayerController)
	SLATE_ARGUMENT(const class UGameplayMainMenuWidgetStyle*, MainMenuStyle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	TWeakObjectPtr<class AGamePlayerController> PlayerController;

	TSharedPtr<class SButton> PlayButton;
	TSharedPtr<class SButton> SpectateButton;
	TSharedPtr<class SButton> ToLobbyButton;

	FReply OnPlayButtonPress();
	FReply OSpectateButtonPress();
	FReply OnBackToLobbyButtonPress();

	void DisableButtons();
};
