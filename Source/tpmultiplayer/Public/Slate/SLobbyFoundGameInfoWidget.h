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
	SLATE_ARGUMENT(const class SLobbyWidget*, ParentLobbyWidget)
	SLATE_ARGUMENT(TWeakObjectPtr<const class ULobbyFoundGameInfoWidgetStyle>, SlateStyle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

protected:
	//TWeakObjectPtr<class SLobbyWidget> ParentLobbyWidget;
};
