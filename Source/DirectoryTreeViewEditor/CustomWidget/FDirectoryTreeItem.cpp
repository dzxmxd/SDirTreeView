#include "FDirectoryTreeItem.h"

FDirectoryTreeItem::FDirectoryTreeItem(const FString& InPath)
	: DiskPath(InPath)
{
	CleanName = FPaths::GetBaseFilename(DiskPath);
	bIsDirectory = !FPaths::FileExists(DiskPath);
}

FDirectoryTreeItem::FDirectoryTreeItem(const FString& InPath, TSharedPtr<FDirectoryTreeItem>& InParent)
	: DiskPath(InPath), Parent(InParent)
{
	CleanName = FPaths::GetBaseFilename(DiskPath);
	bIsDirectory = !FPaths::FileExists(DiskPath);
}

FSimpleMulticastDelegate& FDirectoryTreeItem::OnRenameRequested()
{
	return RenameRequestedEvent;
}
