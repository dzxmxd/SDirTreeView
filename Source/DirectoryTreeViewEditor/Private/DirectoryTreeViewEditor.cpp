// Copyright Epic Games, Inc. All Rights Reserved.

#include "DirectoryTreeViewEditor.h"
#include "DirectoryTreeViewEditorStyle.h"
#include "DirectoryTreeViewEditorCommands.h"
#include "IMainFrameModule.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "CustomWidget/SDirectoryTreeView.h"

static const FName DirectoryTreeViewEditorTabName("DirectoryTreeViewEditor");

#define LOCTEXT_NAMESPACE "FDirectoryTreeViewEditorModule"

void FDirectoryTreeViewEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FDirectoryTreeViewEditorStyle::Initialize();
	FDirectoryTreeViewEditorStyle::ReloadTextures();

	FDirectoryTreeViewEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FDirectoryTreeViewEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FDirectoryTreeViewEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDirectoryTreeViewEditorModule::RegisterMenus));
}

void FDirectoryTreeViewEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FDirectoryTreeViewEditorStyle::Shutdown();

	FDirectoryTreeViewEditorCommands::Unregister();
}

void FDirectoryTreeViewEditorModule::PluginButtonClicked()
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title( NSLOCTEXT("UnrealEd", "WxdTestTitle", "WxdTestTitle") )
		.ClientSize(FVector2D(500,1000));

	FString DirPath(FPaths::ProjectDir() + "../WxdTestFolder/");
	Window->SetContent(
		SNew(SDirectoryTreeView)
		.RootPath(DirPath)
		.OnGetRowCustomToolTips_Raw(this, &FDirectoryTreeViewEditorModule::GetItemCustomToolTips)
	);
	
	IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
	TSharedPtr<SWindow> ParentWindow = MainFrame.GetParentWindow();
	FSlateApplication::Get().AddWindowAsNativeChild(Window, ParentWindow.ToSharedRef());
}

void FDirectoryTreeViewEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FDirectoryTreeViewEditorCommands::Get().PluginAction, PluginCommands, TAttribute<FText>(),
			TAttribute<FText>(),TAttribute<FSlateIcon>(),NAME_None, FName("DirectoryTreeView"));
		}
	}
	
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FDirectoryTreeViewEditorCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

TSharedPtr<IToolTip> FDirectoryTreeViewEditorModule::GetItemCustomToolTips(const TSharedPtr<FDirectoryTreeItem> Item)
{
	return SNew(SToolTip)
	[
		SNew(SButton)
		.Text(LOCTEXT("TestButton", "测试按钮"))
	];
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDirectoryTreeViewEditorModule, DirectoryTreeViewEditor)