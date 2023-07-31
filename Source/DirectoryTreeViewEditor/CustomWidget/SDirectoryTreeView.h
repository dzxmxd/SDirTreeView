#pragma once

struct FFileChangeData;
class FDirectoryTreeItem;
typedef TSharedPtr<FDirectoryTreeItem> FDirTreeInstanceItem;
typedef TWeakPtr<FDirectoryTreeItem> FDirTreeWeakInstanceItem;
typedef STreeView<FDirTreeInstanceItem> SDirTreeViewPtr;

DECLARE_DELEGATE_RetVal_OneParam(TSharedPtr<IToolTip>, FOnGetRowCustomToolTips, const TSharedPtr<FDirectoryTreeItem>);

/* 为方便文章理解，此类中方法、变量按照文章逻辑排列，切勿参考，可自行按职责组织 */
class DIRECTORYTREEVIEWEDITOR_API SDirectoryTreeView : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDirectoryTreeView) :
			_RootPath(""),
			_SelectionMode(ESelectionMode::Single)
			{
			}
	
	/* 依赖的物理根路径 */
	SLATE_ARGUMENT(FString, RootPath)
	
	/* 节点如何被选中 */
	SLATE_ATTRIBUTE(ESelectionMode::Type, SelectionMode)

	/* 创建自定义节点悬浮提示函数 */
	SLATE_EVENT(FOnGetRowCustomToolTips, OnGetRowCustomToolTips)
	
	SLATE_END_ARGS()

public:
	virtual void Construct(const FArguments& InArgs);
	virtual ~SDirectoryTreeView() override;

private:
	/* 获取合法的物理路径 */
	FString GetLegalPath(const FString& InRootPath);
	
	/* 根据根目录路径初始化树形节点 */
	void InitTreeView(const FString& InRootPath);
	
	/* 递归的构造树形结构 */
	void ConstructChildrenRecursively(FDirTreeInstanceItem TreeItem);
	
	/* 具体创建 TreeItem UI 的函数 */
	TSharedRef<ITableRow> GenerateTreeRow(FDirTreeInstanceItem TreeItem, const TSharedRef<STableViewBase>& OwnerTable);
	
	/* 获取某个节点的子节点信息 */
	void GetChildrenForTree(FDirTreeInstanceItem TreeItem, TArray< FDirTreeInstanceItem >& OutChildren);

#pragma region 搜索框
	
	/* 重建目录树 */
	void RebuildDirTreeView();

	/* 使用筛选文字构造树形结构 */
	void ConstructTreeByFilterText(FString DiskPath, const FText& InFilterText);
	
	/* 筛选框文字变动 */
	void OnFilterTextChanged(const FText& InFilterText);
	
#pragma endregion //搜索框

	
#pragma region 双击回调
	
	/* 双击回调 */
	void OnMouseDoubleClicked(TSharedPtr<FDirectoryTreeItem> Item);
	
#pragma endregion // 双击回调

	
#pragma region 右键菜单
	
	/* 右键菜单 */
	TSharedPtr<SWidget> OpenContextMenu();
	
	/* 创建文件夹右键菜单 */
	void CreateFolderMenu(FMenuBuilder& MenuBuilder);
	
	/* 创建文件右键菜单 */
	void CreateFileMenu(FMenuBuilder& MenuBuilder);

	/* 新建文件夹回调 */
	void OnNewFolderClicked();

	/* 获取自增新建文件夹路径 */
	FString GetValidNewFolderPath(FString ParentFolderPath);

	/* 删除文件回调 */
	void OnDeleteFileClicked();
	
#pragma endregion // 右键菜单

#pragma region 拓展能力给使用者
	
	/* 自定义节点悬浮提示代理 */
	FOnGetRowCustomToolTips OnGetRowCustomToolTips;
	
#pragma endregion

#pragma region 重命名逻辑

	void OnRenameFolderClicked();

	void FolderNameChanged(const TSharedPtr<FDirectoryTreeItem>& Item, const FString& ProposedName,
					   const FVector2D& MessageLocation, const ETextCommit::Type CommitType);
	
#pragma endregion

#pragma region 版本控制

	/* 强制更新根目录版本控制状态 */
	void UpdatePathFilesSCCState();
	
#pragma endregion 

#pragma region 监听本地文件变动

	/* 本地目录变更 */
	void OnDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	FDelegateHandle WatcherDelegate;

	void UnbindWatcher();
	
#pragma endregion

#pragma region 展开折叠状态
public:
	void OnExpansionChanged(TSharedPtr<FDirectoryTreeItem> Item, bool bIsExpanded);
#pragma endregion 
	
private:
	TSharedPtr<SDirTreeViewPtr> TreeViewPtr;
	TArray<FDirTreeInstanceItem> TreeRootItems;
	TSet<FString> ExpansionItemPath;
	FString CurrentRootPath;
};