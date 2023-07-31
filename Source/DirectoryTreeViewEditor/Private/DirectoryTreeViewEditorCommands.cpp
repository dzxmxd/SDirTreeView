// Copyright Epic Games, Inc. All Rights Reserved.

#include "DirectoryTreeViewEditorCommands.h"

#define LOCTEXT_NAMESPACE "FDirectoryTreeViewEditorModule"

void FDirectoryTreeViewEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "DirectoryTreeViewEditor", "Execute DirectoryTreeViewEditor action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
