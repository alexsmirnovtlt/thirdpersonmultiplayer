// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Main widget for lobby, where player is able to host a game or join one that was already created
 */
class TPMULTIPLAYER_API SLobbyWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLobbyWidget)
	{}
	SLATE_ARGUMENT(TWeakObjectPtr<class ALobbyGameMode>, LobbyGameMode)
	SLATE_ARGUMENT(struct FLobbyMenuSlateStyle*, LobbyStyle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	void OnSessionNameChanged(const FText& InText, ETextCommit::Type);
	void OnPlayerNameChanged(const FText& InText, ETextCommit::Type);

	FReply OnHostButtonClick();
	FReply OnSearchButtonClick();
	FReply OnJoinButtonClick();

	FText SessionName;
	FText PlayerName;

	TSharedPtr<class STextBlock> WaitingTextBlock;

	TSharedPtr<class SVerticalBox> FoundGamesVerticalBox;

	TSharedPtr<class SButton> HostButton;
	TSharedPtr<class SButton> JoinButton;
	TSharedPtr<class SButton> SearchButton;

	TWeakObjectPtr<class ALobbyGameMode> LobbyGameMode;
};
