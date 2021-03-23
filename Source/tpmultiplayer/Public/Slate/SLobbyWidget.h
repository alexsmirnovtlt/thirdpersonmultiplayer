// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "General/GameModes/LobbyGameMode.h"

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

//private:
	//TWeakObjectPtr<class ALobbyGameMode> LobbyGameMode;
};
