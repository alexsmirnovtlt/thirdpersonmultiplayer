// Aleksandr Smirnov 2021


#include "Slate/SLobbyWidget.h"
#include "SlateOptMacros.h"

#include "Widgets/Input/SEditableText.h"

#define LOCTEXT_NAMESPACE "LobbyWidget"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLobbyWidget::Construct(const FArguments& InArgs)
{
	auto Styles = InArgs._LobbyStyle;

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SOverlay)
		
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(300.f, 100.f, 300.f, 100.f)
		[
			SNew(SHorizontalBox)

			// Left Side - Games list Scroll
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SButton)
			]

			// Right Side - Buttons and TextEdit
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.Padding(30.f, 0, 0, 0)
			[
				SNew(SVerticalBox)

				+SVerticalBox::Slot()
				[
					SNew(SBox)
					.WidthOverride(300.f)
					.HeightOverride(50.f)
					[
						SNew(SEditableText)
						.Style(&Styles->EditableTextStyle)
						.Text(LOCTEXT("LobbyWidget.CHANGEME", "CHANGE ME"))
					]
				]

				+ SVerticalBox::Slot()
				.Padding(0, 30.f, 0, 0)
				[
					SNew(SBox)
					.WidthOverride(300.f)
					.HeightOverride(50.f)
					[
						SNew(SButton)
						.ContentPadding(FMargin(10.f, 0, 10.f, 0))
						.Content()
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.TextStyle(&Styles->MainButtonTextStyle)
							.Text(LOCTEXT("LobbyWidget.CHANGEME", "CHANGE ME"))
						]
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE