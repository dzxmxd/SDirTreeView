// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "DirectoryTreeViewEditorStyle.h"

class FDirectoryTreeViewEditorCommands : public TCommands<FDirectoryTreeViewEditorCommands>
{
public:

	FDirectoryTreeViewEditorCommands()
		: TCommands<FDirectoryTreeViewEditorCommands>(TEXT("DirectoryTreeViewEditor"), NSLOCTEXT("Contexts", "DirectoryTreeViewEditor", "DirectoryTreeViewEditor Plugin"), NAME_None, FDirectoryTreeViewEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
