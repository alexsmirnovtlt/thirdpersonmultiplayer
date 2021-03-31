// Aleksandr Smirnov 2021


#include "Slate/GameplayMainHUDWidgetStyle.h"

FGameplayMainHUDStyle::FGameplayMainHUDStyle()
{
}

FGameplayMainHUDStyle::~FGameplayMainHUDStyle()
{
}

const FName FGameplayMainHUDStyle::TypeName(TEXT("FGameplayMainHUDStyle"));

const FGameplayMainHUDStyle& FGameplayMainHUDStyle::GetDefault()
{
	static FGameplayMainHUDStyle Default;
	return Default;
}

void FGameplayMainHUDStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
}

