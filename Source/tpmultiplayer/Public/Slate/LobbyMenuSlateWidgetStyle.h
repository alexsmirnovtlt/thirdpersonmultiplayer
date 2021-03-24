// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "LobbyMenuSlateWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct TPMULTIPLAYER_API FLobbyMenuSlateStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FLobbyMenuSlateStyle();
	virtual ~FLobbyMenuSlateStyle();

	// Custom properties
	UPROPERTY(EditAnywhere, Category = "Background")
	FSlateColor BackgroundColor;
	UPROPERTY(EditAnywhere, Category = "Background")
	FSlateBrush BackgroundBrush;
	
	UPROPERTY(EditAnywhere, Category = "Editable Text")
	FName EditableTextDefaultText;
	UPROPERTY(EditAnywhere, Category = "Editable Text")
	FSlateColor EditableTextBackgroundColor;
	UPROPERTY(EditAnywhere, Category = "Editable Text")
	FEditableTextStyle EditableTextStyle;
	UPROPERTY(EditAnywhere, Category = "Editable Text")
	FSlateBrush EditableTextBackgroundBrush;
	UPROPERTY(EditAnywhere, Category = "Editable Text")
	FSlateBrush EditableTextCaretBrush;

	UPROPERTY(EditAnywhere, Category = "Button")
	FTextBlockStyle ButtonTextStyle;

	const FText GetEditTextDefaultText() const { return FText::FromName(EditableTextDefaultText); };

	//

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FLobbyMenuSlateStyle& GetDefault();
};

/**
 */
UCLASS(Blueprintable, abstract, hidecategories=Object, MinimalAPI)
class ULobbyMenuSlateWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:

	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FLobbyMenuSlateStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};
