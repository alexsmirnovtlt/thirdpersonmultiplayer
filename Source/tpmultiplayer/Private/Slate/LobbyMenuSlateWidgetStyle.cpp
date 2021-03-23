// Aleksandr Smirnov 2021


#include "Slate/LobbyMenuSlateWidgetStyle.h"

FLobbyMenuSlateStyle::FLobbyMenuSlateStyle()
{
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
	// Add any brush resources here so that Slate can correctly atlas and reference them
}

