// Aleksandr Smirnov 2021


#include "Slate/LobbyFoundGameInfoWidgetStyle.h"

FLobbyFoundGameInfoStyle::FLobbyFoundGameInfoStyle()
{
}

FLobbyFoundGameInfoStyle::~FLobbyFoundGameInfoStyle()
{
}

const FName FLobbyFoundGameInfoStyle::TypeName(TEXT("FLobbyFoundGameInfoStyle"));

const FLobbyFoundGameInfoStyle& FLobbyFoundGameInfoStyle::GetDefault()
{
	static FLobbyFoundGameInfoStyle Default;
	return Default;
}

void FLobbyFoundGameInfoStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	Super::GetResources(OutBrushes);

	OutBrushes.Add(&BackgroundDefautBrush);
}

