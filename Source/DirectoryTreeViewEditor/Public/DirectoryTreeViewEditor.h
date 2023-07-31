// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidget/FDirectoryTreeItem.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FDirectoryTreeViewEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();
	
	TSharedPtr<IToolTip> GetItemCustomToolTips(const TSharedPtr<FDirectoryTreeItem> Item);


private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
