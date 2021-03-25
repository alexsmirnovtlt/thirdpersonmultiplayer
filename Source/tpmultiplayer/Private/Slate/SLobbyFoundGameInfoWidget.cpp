// Aleksandr Smirnov 2021


#include "Slate/SLobbyFoundGameInfoWidget.h"

#include "SlateOptMacros.h"

#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"

#include "Slate/SLobbyWidget.h"
#include "Slate/LobbyFoundGameInfoWidgetStyle.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLobbyFoundGameInfoWidget::Construct(const FArguments& InArgs)
{
	if (!InArgs._SlateStyle.IsValid()) return;

	const auto& Style = InArgs._SlateStyle.Get()->WidgetStyle;

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	.Padding(0, 0, 0, 15.f)
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SImage)
			.ColorAndOpacity(Style.BackgroundDefaultColor)
			.Image(&Style.BackgroundDefautBrush)
		]
		/*+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			//SNew(SButton)
		]*/
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
