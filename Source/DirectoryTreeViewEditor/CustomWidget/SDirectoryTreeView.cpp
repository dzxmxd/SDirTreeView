#include "SDirectoryTreeView.h"

#include "DirectoryWatcherModule.h"
#include "FDirectoryTreeItem.h"
#include "IDirectoryWatcher.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SDirectoryTreeItem.h"
#include "SourceControlOperations.h"
#include "SSearchBox.h"

#define LOCTEXT_NAMESPACE "DirectoryTreeView"

/**
 * SDirectoryTreeView
 */
SDirectoryTreeView::~SDirectoryTreeView()
{
	this->UnbindWatcher();
}

void SDirectoryTreeView::Construct(const FArguments& InArgs)
{
	this->CurrentRootPath = this->GetLegalPath(InArgs._RootPath);
	this->InitTreeView(this->CurrentRootPath);
	this->OnGetRowCustomToolTips = InArgs._OnGetRowCustomToolTips;

	SAssignNew(this->TreeViewPtr, SDirTreeViewPtr)
	.TreeItemsSource(&this->TreeRootItems)
	.ClearSelectionOnClick(false)
	.SelectionMode(InArgs._SelectionMode)
	.HighlightParentNodesForSelection(true)
	.OnGenerateRow(this, &SDirectoryTreeView::GenerateTreeRow)
	.OnGetChildren(this, &SDirectoryTreeView::GetChildrenForTree)
	.OnMouseButtonDoubleClick(this, &SDirectoryTreeView::OnMouseDoubleClicked)
	.OnContextMenuOpening(this, &SDirectoryTreeView::OpenContextMenu)
	.OnExpansionChanged(this, &SDirectoryTreeView::OnExpansionChanged)
	;
	
	this->UpdatePathFilesSCCState();

	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>("DirectoryWatcher");
	IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get();
	if (DirectoryWatcher != nullptr)
	{
		IDirectoryWatcher::FDirectoryChanged Callback = IDirectoryWatcher::FDirectoryChanged::CreateRaw(
			this, &SDirectoryTreeView::OnDirectoryChanged);
		DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(this->CurrentRootPath, Callback, this->WatcherDelegate,
																  IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges);
	}
	
	this->ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSearchBox)
				.ToolTipText(LOCTEXT("FilterSearchToolTip", "Type here to search file"))
				.HintText(LOCTEXT("FilterSearchHint", "Search File"))
				.OnTextChanged(this, &SDirectoryTreeView::OnFilterTextChanged)
		]
		+ SVerticalBox::Slot()
		[
			this->TreeViewPtr.ToSharedRef()
		]
	];
}

FString SDirectoryTreeView::GetLegalPath(const FString& InRootPath)
{
	if (InRootPath.IsEmpty())
	{
		return FPaths::ProjectContentDir();
	}
	FString Result = InRootPath;
	Result = Result.Replace(TEXT("\\"), TEXT("/"));
	Result.RemoveFromEnd("/");
	return Result;
}

void SDirectoryTreeView::InitTreeView(const FString& InRootPath)
{
	FDirTreeInstanceItem Root = FDirTreeInstanceItem(new FDirectoryTreeItem(InRootPath));
	this->TreeRootItems.Add(Root);
	this->ConstructChildrenRecursively(Root);
}

void SDirectoryTreeView::ConstructChildrenRecursively(FDirTreeInstanceItem TreeItem)
{
	if (this->ExpansionItemPath.Find(TreeItem->DiskPath))
	{
		this->TreeViewPtr->SetItemExpansion(TreeItem, true);
	}
	if (TreeItem.IsValid())
	{
		TArray<FString> FindedFiles;
		FString SearchFile = TreeItem->DiskPath + "/*.*";
		IFileManager::Get().FindFiles(FindedFiles, *SearchFile, true, true);

		for (auto& Element : FindedFiles)
		{
			FString FullPath = TreeItem->DiskPath + "/" + Element;
			FDirTreeInstanceItem Child = FDirTreeInstanceItem(new FDirectoryTreeItem(FullPath, TreeItem));
			TreeItem->Children.Add(Child);
			if (Child->bIsDirectory)
			{
				this->ConstructChildrenRecursively(Child);
			}
		}
	}
}

TSharedRef<ITableRow> SDirectoryTreeView::GenerateTreeRow(FDirTreeInstanceItem TreeItem,
                                                          const TSharedRef<STableViewBase>& OwnerTable)
{
	check(TreeItem.IsValid());
	TSharedPtr<IToolTip> ItemToolTip = SNew(SToolTip);
	if (this->OnGetRowCustomToolTips.IsBound())
	{
		ItemToolTip = this->OnGetRowCustomToolTips.Execute(TreeItem);
	}
	return
		SNew(STableRow<FDirTreeInstanceItem>, OwnerTable)
		.ToolTip(ItemToolTip)
		[
			SNew(SDirectoryTreeItem)
			.TreeItem(TreeItem)
			.OnNameChanged(this, &SDirectoryTreeView::FolderNameChanged)
		];
}

void SDirectoryTreeView::GetChildrenForTree(FDirTreeInstanceItem TreeItem, TArray<FDirTreeInstanceItem>& OutChildren)
{
	//获取TreeItem的子节点信息
	if (TreeItem.IsValid())
	{
		OutChildren = TreeItem->Children;
	}
}

void SDirectoryTreeView::RebuildDirTreeView()
{
	this->TreeRootItems.Empty();
	this->InitTreeView(CurrentRootPath);
	this->TreeViewPtr->RequestTreeRefresh();
}

void SDirectoryTreeView::ConstructTreeByFilterText(FString DiskPath, const FText& InFilterText)
{
	TArray<FString> FindedFiles;
	FString SearchFile = DiskPath + "/*.*";
	IFileManager::Get().FindFiles(FindedFiles, *SearchFile, true, true);
	for (auto& Element : FindedFiles)
	{
		FString ItemPath = DiskPath + "/" + Element;
		FDirTreeInstanceItem Child = FDirTreeInstanceItem(new FDirectoryTreeItem(ItemPath));
		if (Child->bIsDirectory)
		{
			this->ConstructTreeByFilterText(ItemPath, InFilterText);
			continue;
		}
		if (!Child->bIsDirectory && Element.Contains(InFilterText.ToString()))
		{
			this->TreeRootItems.Add(Child);
		}
	}
}

void SDirectoryTreeView::OnFilterTextChanged(const FText& InFilterText)
{
	if (InFilterText.IsEmpty())
	{
		this->RebuildDirTreeView();
		return ;
	}
	this->TreeRootItems.Empty();
	this->ConstructTreeByFilterText(CurrentRootPath, InFilterText);
	this->TreeViewPtr->RequestTreeRefresh();
}

void SDirectoryTreeView::OnMouseDoubleClicked(TSharedPtr<FDirectoryTreeItem> Item)
{
	if (Item->bIsDirectory)
	{
		this->TreeViewPtr->SetItemExpansion(Item, !this->TreeViewPtr->IsItemExpanded(Item));
		UE_LOG(LogTemp, Log, TEXT("SDirectoryTreeView, Mouse button double clicked."));
	}
}

TSharedPtr<SWidget> SDirectoryTreeView::OpenContextMenu()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, NULL);
	if (this->TreeViewPtr->GetSelectedItems().IsEmpty())
	{
		return MenuBuilder.MakeWidget();
	}
	if (this->TreeViewPtr->GetSelectedItems()[0]->bIsDirectory)
	{
		this->CreateFolderMenu(MenuBuilder);
	}
	else
	{
		this->CreateFileMenu(MenuBuilder);
	}
	return MenuBuilder.MakeWidget();
}

void SDirectoryTreeView::CreateFolderMenu(FMenuBuilder& MenuBuilder)
{

	MenuBuilder.AddMenuEntry(
		LOCTEXT("NewFolder", "新建文件夹"),
		LOCTEXT("NewFolderTooltip", "在当前目录下新建文件夹."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.NewFolderIcon"),
		FUIAction(FExecuteAction::CreateSP(this, &SDirectoryTreeView::OnNewFolderClicked))
	);

	MenuBuilder.AddMenuEntry(
	LOCTEXT("RenameFolder", "重命名文件夹"),
	LOCTEXT("RenameTooltip", "重命名此文件夹."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Rename"),
	FUIAction(FExecuteAction::CreateSP(this, &SDirectoryTreeView::OnRenameFolderClicked))
);
	
}

void SDirectoryTreeView::CreateFileMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
	LOCTEXT("DeleteFile", "删除"),
	LOCTEXT("DeleteFileTooltip", "删除此文件."),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Delete"),
	FUIAction(FExecuteAction::CreateSP(this, &SDirectoryTreeView::OnDeleteFileClicked))
	);
}

void SDirectoryTreeView::OnNewFolderClicked()
{
	FDirTreeInstanceItem Item = this->TreeViewPtr->GetSelectedItems()[0];
	FString FolderPath = GetValidNewFolderPath(Item->DiskPath);
	bool State = IFileManager::Get().MakeDirectory(*FolderPath, true);
	UE_LOG(LogTemp, Log, TEXT("SDirectoryTreeView, Create %s"), *(FolderPath + (State ? " Success" : " Fail")));	
}

FString SDirectoryTreeView::GetValidNewFolderPath(FString ParentFolderPath)
{
	FString FolderName = TEXT("/新建文件夹");
	FString NewFolderPath = ParentFolderPath + FolderName;
	for (int32 Suffix = 1; IFileManager::Get().DirectoryExists(*NewFolderPath); Suffix++)
	{
		NewFolderPath = ParentFolderPath + FolderName + FString::FromInt(Suffix);
	}
	return NewFolderPath;
}

void SDirectoryTreeView::OnDeleteFileClicked()
{
	FDirTreeInstanceItem Item = this->TreeViewPtr->GetSelectedItems()[0];
	FString FilePath = Item->DiskPath;
	bool State = IFileManager::Get().Delete(*FilePath, true, true);
	UE_LOG(LogTemp, Log, TEXT("SDirectoryTreeView, Delete %s"), *(FilePath + (State ? " Success" : " Fail")));
}

void SDirectoryTreeView::OnRenameFolderClicked()
{
	FDirTreeInstanceItem Item = this->TreeViewPtr->GetSelectedItems()[0];
	if (TreeRootItems.Contains(Item))
	{
		return;
	}
	Item->OnRenameRequested().Broadcast();
}

void SDirectoryTreeView::FolderNameChanged(const TSharedPtr<FDirectoryTreeItem>& Item, const FString& ProposedName,
	const FVector2D& MessageLocation, const ETextCommit::Type CommitType)
{
	FString FolderPath = FPaths::GetPath(Item->DiskPath) + "/" + ProposedName;
	if (IFileManager::Get().DirectoryExists(*FolderPath))
	{
		return;
	}
	bool State = IFileManager::Get().Move(*FolderPath, *Item->DiskPath, true, true);
	Item->CleanName = ProposedName;
	this->TreeViewPtr->RequestTreeRefresh();
	UE_LOG(LogTemp, Log, TEXT("SDirTreeView, Rename %s"), *(FolderPath +  + (State ? " Success" : " Fail")));
}

void SDirectoryTreeView::UpdatePathFilesSCCState()
{
	TArray<FString> FilesAddToSCC;
	FString SearchFile = "*.*";
	IFileManager::Get().FindFilesRecursive(FilesAddToSCC, *this->CurrentRootPath, *SearchFile, true, false);
	
	ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
	TSharedPtr<FUpdateStatus, ESPMode::ThreadSafe> UpdateStatusOp = ISourceControlOperation::Create<FUpdateStatus>();
	UpdateStatusOp->SetForceUpdate(true);
	SourceControlProvider.Execute(UpdateStatusOp.ToSharedRef(), FilesAddToSCC, EConcurrency::Asynchronous);
}

void SDirectoryTreeView::OnDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	this->RebuildDirTreeView();
}

void SDirectoryTreeView::UnbindWatcher()
{
	if (!this->WatcherDelegate.IsValid())
	{
		return;
	}
	if (FDirectoryWatcherModule* Module = FModuleManager::GetModulePtr<FDirectoryWatcherModule>(TEXT("DirectoryWatcher")))
	{
		if (IDirectoryWatcher* DirectoryWatcher = Module->Get())
		{
			DirectoryWatcher->UnregisterDirectoryChangedCallback_Handle(this->CurrentRootPath, this->WatcherDelegate);
		}
	}
	this->WatcherDelegate.Reset();
}

void SDirectoryTreeView::OnExpansionChanged(TSharedPtr<FDirectoryTreeItem> Item, bool bIsExpanded)
{
	if (bIsExpanded)
	{
		this->ExpansionItemPath.Add(Item->DiskPath);
	}
	else
	{
		this->ExpansionItemPath.Remove(Item->DiskPath);
	}
}

#undef LOCTEXT_NAMESPACE
