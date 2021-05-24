// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "GameplayMainMenuWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct TPMULTIPLAYER_API FGameplayMainMenuStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FGameplayMainMenuStyle();
	virtual ~FGameplayMainMenuStyle();

	UPROPERTY(EditAnywhere, Category = "Default")
	FSlateColor BackgroundColor;
	UPROPERTY(EditAnywhere, Category = "Default")
	FSlateBrush BackgroundBrush;

	UPROPERTY(EditAnywhere, Category = "Default")
	FButtonStyle ButtonStyle;

	UPROPERTY(EditAnywhere, Category = "Default")
	FTextBlockStyle TextStyle;

	UPROPERTY(EditAnywhere, Category = "Default")
	FTextBlockStyle HintTextStyle;

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FGameplayMainMenuStyle& GetDefault();
};

/**
 */
UCLASS(Blueprintable, abstract, hidecategories=Object, MinimalAPI)
class UGameplayMainMenuWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FGameplayMainMenuStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};
