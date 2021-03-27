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
	SLATE_ARGUMENT(const class ULobbyMenuSlateWidgetStyle*, LobbyStyle)
	SLATE_ARGUMENT(TWeakObjectPtr<class ULobbyFoundGameInfoWidgetStyle>, SessionItemStyle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetButtonEnabled_Host(bool bIsEnabled);
	void SetButtonEnabled_Join(bool bIsEnabled);
	void SetButtonEnabled_Search(bool bIsEnabled);
	void SetButtonsEnabled(bool bHost, bool bJoin, bool bSearch);

	void ClearSessionsList();
	void DisplayNoSessionsFound();
	void ResetButtonState();
	void AddFoundSession(FString& SessionNameStr, int32 CurrentPlayersCount, int32 MaxPlayersCount, int32 Index);
	void OnSessionItemSelected(int32 index);
	const FString& GetPlayerName() const;

private:

	void OnSessionNameChanged(const FText& InText, ETextCommit::Type);
	void OnPlayerNameChanged(const FText& InText, ETextCommit::Type);

	FReply OnHostButtonClick();
	FReply OnSearchButtonClick();
	FReply OnJoinButtonClick();

	FText SessionName;
	FText PlayerName;

	TWeakObjectPtr<const class ULobbyFoundGameInfoWidgetStyle> SessionItemStyle;

	TSharedPtr<class STextBlock> WaitingTextBlock;
	TSharedPtr<class STextBlock> NothingFoundTextBlock;

	TSharedPtr<class SVerticalBox> FoundGamesVerticalBox;

	TSharedPtr<class SButton> HostButton;
	TSharedPtr<class SButton> JoinButton;
	TSharedPtr<class SButton> SearchButton;

	TWeakObjectPtr<class ALobbyGameMode> LobbyGameMode;

	TArray <TSharedRef<class SLobbyFoundGameInfoWidget>> RunningGamesArray;

	int32 ChosenSessionIndex = -1;
};
