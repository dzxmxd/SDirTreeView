// Copyright Epic Games, Inc. All Rights Reserved.

#include "DirectoryTreeViewEditorStyle.h"
#include "DirectoryTreeViewEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FDirectoryTreeViewEditorStyle::StyleInstance = nullptr;

void FDirectoryTreeViewEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDirectoryTreeViewEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FDirectoryTreeViewEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DirectoryTreeViewEditorStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FDirectoryTreeViewEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("DirectoryTreeViewEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("DirectoryTreeViewEditor")->GetBaseDir() / TEXT("Resources"));

	Style->Set("DirectoryTreeViewEditor.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FDirectoryTreeViewEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FDirectoryTreeViewEditorStyle::Get()
{
	return *StyleInstance;
}
