// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class TPMULTIPLAYER_API SGameplayMainHUDWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameplayMainHUDWidget)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
