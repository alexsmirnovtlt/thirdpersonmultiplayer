// Aleksandr Smirnov 2021


#include "Slate/Styles/GameplayMainMenuWidgetStyle.h"

FGameplayMainMenuStyle::FGameplayMainMenuStyle()
{
}

FGameplayMainMenuStyle::~FGameplayMainMenuStyle()
{
}

const FName FGameplayMainMenuStyle::TypeName(TEXT("FGameplayMainMenuStyle"));

const FGameplayMainMenuStyle& FGameplayMainMenuStyle::GetDefault()
{
	static FGameplayMainMenuStyle Default;
	return Default;
}

void FGameplayMainMenuStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&BackgroundBrush);
}

