// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


/**
 * Info about currently running game: session name and players count.
 * Those widgets populate scroll of all found games returned by online sybsystem
 */
class TPMULTIPLAYER_API SLobbyFoundGameInfoWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLobbyFoundGameInfoWidget)
	{}
	SLATE_ARGUMENT(class SLobbyWidget*, ParentLobbyWidget)
	SLATE_ARGUMENT(const class ULobbyFoundGameInfoWidgetStyle*, SlateStyle)
	SLATE_ARGUMENT(FString, SessionNameStr)
	SLATE_ARGUMENT(int32, CurrentPlayersCount)
	SLATE_ARGUMENT(int32, MaxPlayersCount)
	SLATE_ARGUMENT(int32, OnlineSubsystemIndex)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Deselect();

protected:

	FReply OnClicked();

	//bool bIsSelected = false;

	TSharedPtr<SButton> Button;

	int32 SubsystemIndex;
	class SLobbyWidget* ParentLobbyWidget; // TODO change from raw pointer

	static const FString SeparatorStr;
};
