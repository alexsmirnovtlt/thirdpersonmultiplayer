// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "LobbyFoundGameInfoWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT()
struct TPMULTIPLAYER_API FLobbyFoundGameInfoStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FLobbyFoundGameInfoStyle();
	virtual ~FLobbyFoundGameInfoStyle();

	UPROPERTY(EditAnywhere, Category = "Default")
	FButtonStyle DefaultButtonStyle;

	UPROPERTY(EditAnywhere, Category = "Default")
	FTextBlockStyle DefaultTextStyle;

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FLobbyFoundGameInfoStyle& GetDefault();
};

/**
 */
UCLASS(Blueprintable, abstract, hidecategories=Object, MinimalAPI)
class ULobbyFoundGameInfoWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FLobbyFoundGameInfoStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		UE_LOG(LogTemp, Warning, TEXT("FSlateWidgetStyle::GetStyle"));
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};
