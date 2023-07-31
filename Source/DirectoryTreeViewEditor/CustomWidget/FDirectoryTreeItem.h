#pragma once

class FDirectoryTreeItem
{
public:
	FDirectoryTreeItem(const FString& InPath);
	FDirectoryTreeItem(const FString& InPath, TSharedPtr<FDirectoryTreeItem>&InParent);
public:
	FSimpleMulticastDelegate& OnRenameRequested();
	
	FString DiskPath;
	FString CleanName;
	bool bIsDirectory;
	TSharedPtr<FDirectoryTreeItem> Parent;
	TArray<TSharedPtr<FDirectoryTreeItem>> Children;

	FSimpleMulticastDelegate RenameRequestedEvent;
};