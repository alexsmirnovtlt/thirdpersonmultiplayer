// Aleksandr Smirnov 2021


#include "Slate/SLobbyFoundGameInfoWidget.h"

#include "SlateOptMacros.h"

#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"

#include "Slate/SLobbyWidget.h"
#include "Slate/Styles/LobbyFoundGameInfoWidgetStyle.h"

const FString SLobbyFoundGameInfoWidget::SeparatorStr("/");

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLobbyFoundGameInfoWidget::Construct(const FArguments& InArgs)
{
	const auto& Style = InArgs._SlateStyle->WidgetStyle;
	
	SubsystemIndex = InArgs._OnlineSubsystemIndex;

	ParentLobbyWidget = InArgs._ParentLobbyWidget;

	FText SessionName = FText::FromString(InArgs._SessionNameStr);
	FString PlayerNumbersStr = FString::FromInt(InArgs._CurrentPlayersCount) + SeparatorStr + FString::FromInt(InArgs._MaxPlayersCount);
	FText PlayerNumbers = FText::FromString(PlayerNumbersStr);

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	.Padding(50.f, 0, 50.f, 15.f)
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		[
			SAssignNew(Button, SButton)
			.OnClicked(this, &SLobbyFoundGameInfoWidget::OnClicked)
			.ButtonStyle(&Style.DefaultButtonStyle)
		]

		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			.Visibility(EVisibility::HitTestInvisible)

			+ SHorizontalBox::Slot()
			.Padding(30.0f, 20.f, 30.f, 20.f)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.TextStyle(&Style.DefaultTextStyle)
				.Justification(ETextJustify::Center)
				.Text(SessionName)
			]

			+ SHorizontalBox::Slot()
			.Padding(30.0f, 20.f, 30.f, 20.f)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.TextStyle(&Style.DefaultTextStyle)
				.Justification(ETextJustify::Center)
				.Text(PlayerNumbers)
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLobbyFoundGameInfoWidget::Deselect()
{
	Button.Get()->SetEnabled(true);
}

FReply SLobbyFoundGameInfoWidget::OnClicked()
{
	if (!ParentLobbyWidget.IsValid()) return FReply::Handled();
	ParentLobbyWidget.Pin()->OnSessionItemSelected(SubsystemIndex);

	Button.Get()->SetEnabled(false);
	return FReply::Handled();
}