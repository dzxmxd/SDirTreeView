#pragma once

class SLayeredImage;
class FDirectoryTreeItem;

class SDirectoryTreeItem : public SCompoundWidget
{
	DECLARE_DELEGATE_FourParams(FOnNameChanged, const TSharedPtr<FDirectoryTreeItem>&, const FString&, const FVector2D&, const ETextCommit::Type);
	DECLARE_DELEGATE_RetVal_ThreeParams(bool, FOnVerifyNameChanged, const TSharedPtr<FDirectoryTreeItem>&, const FString&, FText&);
	
public:
	SLATE_BEGIN_ARGS(SDirectoryTreeItem)
			: _TreeItem(TSharedPtr<FDirectoryTreeItem>())
		{
		}

		SLATE_ARGUMENT(TSharedPtr<FDirectoryTreeItem>, TreeItem)
		SLATE_EVENT(FOnNameChanged, OnNameChanged)
		SLATE_EVENT(FOnVerifyNameChanged, OnVerifyNameChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SDirectoryTreeItem();

private:
	FText GetNameText() const;
	TSharedPtr<SWidget> CreateIcon() const;

	void HandleNameCommitted(const FText& NewText, ETextCommit::Type /*CommitInfo*/);

	void HandleSourceControlProviderChanged(class ISourceControlProvider& OldProvider, class ISourceControlProvider& NewProvider);
	void HandleSourceControlStateChanged();

private:
	TWeakPtr<FDirectoryTreeItem> TreeItem;
	
	FOnNameChanged OnNameChanged;
	TSharedPtr<SInlineEditableTextBlock> InlineRenameWidget;
	FDelegateHandle EnterEditingModeDelegateHandle;
	
	TSharedPtr<SLayeredImage> SCCStateWidget;
	FDelegateHandle SourceControlStateChangedDelegateHandle;
};
