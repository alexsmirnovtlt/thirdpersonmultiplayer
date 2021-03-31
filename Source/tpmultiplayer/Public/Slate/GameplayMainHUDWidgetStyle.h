// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "GameplayMainHUDWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct TPMULTIPLAYER_API FGameplayMainHUDStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FGameplayMainHUDStyle();
	virtual ~FGameplayMainHUDStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FGameplayMainHUDStyle& GetDefault();
};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UGameplayMainHUDWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FGameplayMainHUDStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};
