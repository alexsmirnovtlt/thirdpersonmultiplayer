// Aleksandr Smirnov 2021


#include "Slate/LobbyMenuSlateWidgetStyle.h"

FLobbyMenuSlateStyle::FLobbyMenuSlateStyle()
{
	SessionName = TEXT("GameSession");
	PlayerName = TEXT("Player1");
}

FLobbyMenuSlateStyle::~FLobbyMenuSlateStyle()
{
}

const FName FLobbyMenuSlateStyle::TypeName(TEXT("FLobbyMenuSlateStyle"));

const FLobbyMenuSlateStyle& FLobbyMenuSlateStyle::GetDefault()
{
	static FLobbyMenuSlateStyle Default;
	return Default;
}

void FLobbyMenuSlateStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	Super::GetResources(OutBrushes);
	
	OutBrushes.Add(&EditableTextBackgroundBrush);
	OutBrushes.Add(&EditableTextCaretBrush);
	OutBrushes.Add(&BackgroundBrush);
}

