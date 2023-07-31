#include "SDirectoryTreeItem.h"
#include "FDirectoryTreeItem.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SInlineEditableTextBlock.h"
#include "SLayeredImage.h"

void SDirectoryTreeItem::Construct(const FArguments& InArgs)
{
	this->TreeItem = InArgs._TreeItem;
	const FSlateBrush* IconBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderOpen");
	const float IconOverlaySize = IconBrush->ImageSize.X * 0.6f;

	this->OnNameChanged = InArgs._OnNameChanged;
	
	this->ChildSlot
	[
		SNew(SBox)
		.WidthOverride(200.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			  .VAlign(VAlign_Center)
			  .AutoWidth()
			  .Padding(0, 0, 4, 0)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					this->CreateIcon().ToSharedRef()
				]
				+ SOverlay::Slot()
				  .HAlign(HAlign_Right)
				  .VAlign(VAlign_Top)
				[
					SNew(SBox)
				.WidthOverride(IconOverlaySize)
				.HeightOverride(IconOverlaySize)
					[
						SAssignNew(this->SCCStateWidget, SLayeredImage)
						.Image(FStyleDefaults::GetNoBrush())
					]
				]
			]
			+ SHorizontalBox::Slot()
			  .VAlign(VAlign_Center)
			  .AutoWidth()
			[
				SAssignNew(this->InlineRenameWidget, SInlineEditableTextBlock)
				.Text(this, &SDirectoryTreeItem::GetNameText)
				.OnTextCommitted(this, &SDirectoryTreeItem::HandleNameCommitted)
				// .OnVerifyTextChanged(this, &SAssetTreeItem::VerifyNameChanged)
			]
		]
	];

	if (this->InlineRenameWidget.IsValid())
	{
		this->EnterEditingModeDelegateHandle = this->TreeItem.Pin()->OnRenameRequested().AddSP(
			this->InlineRenameWidget.Get(), &SInlineEditableTextBlock::EnterEditingMode);
	}

	ISourceControlModule::Get().RegisterProviderChanged(FSourceControlProviderChanged::FDelegate::CreateSP(this, &SDirectoryTreeItem::HandleSourceControlProviderChanged));
	SourceControlStateChangedDelegateHandle = ISourceControlModule::Get().GetProvider().RegisterSourceControlStateChanged_Handle(FSourceControlStateChanged::FDelegate::CreateSP(this, &SDirectoryTreeItem::HandleSourceControlStateChanged));
	
	this->HandleSourceControlStateChanged();
}

SDirectoryTreeItem::~SDirectoryTreeItem()
{
	if (this->InlineRenameWidget.IsValid())
	{
		this->TreeItem.Pin()->OnRenameRequested().Remove(this->EnterEditingModeDelegateHandle);
	}
}

FText SDirectoryTreeItem::GetNameText() const
{
	if (TSharedPtr<FDirectoryTreeItem> TreeItemPin = this->TreeItem.Pin())
	{
		return FText::FromString(TreeItemPin->CleanName);
	}
	return FText();
}

TSharedPtr<SWidget> SDirectoryTreeItem::CreateIcon() const
{
	FSlateColor IconColor = FSlateColor::UseForeground();
	const FSlateBrush* Brush = FAppStyle::GetBrush(this->TreeItem.Pin()->bIsDirectory
		                                               ? "ContentBrowser.AssetTreeFolderOpen"
		                                               : "ContentBrowser.ColumnViewAssetIcon");
	return SNew(SImage).Image(Brush).ColorAndOpacity(IconColor);
}

void SDirectoryTreeItem::HandleNameCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	if (this->TreeItem.IsValid())
	{
		TSharedPtr<FDirectoryTreeItem> TreeItemPtr = TreeItem.Pin();
		const FGeometry LastGeometry = GetTickSpaceGeometry();
		FVector2D MessageLoc;
		MessageLoc.X = LastGeometry.AbsolutePosition.X;
		MessageLoc.Y = LastGeometry.AbsolutePosition.Y + LastGeometry.Size.Y * LastGeometry.Scale;
		this->OnNameChanged.ExecuteIfBound(TreeItemPtr, NewText.ToString(), MessageLoc, CommitInfo);
	}
}

void SDirectoryTreeItem::HandleSourceControlProviderChanged(ISourceControlProvider& OldProvider,
                                                            ISourceControlProvider& NewProvider)
{
	OldProvider.UnregisterSourceControlStateChanged_Handle(SourceControlStateChangedDelegateHandle);
	SourceControlStateChangedDelegateHandle = NewProvider.RegisterSourceControlStateChanged_Handle(FSourceControlStateChanged::FDelegate::CreateSP(this, &SDirectoryTreeItem::HandleSourceControlStateChanged));
	// Reset this so the state will be queried from the new provider on the next Tick
	// SourceControlStateDelay = 0.0f;
	// bSourceControlStateRequested = false;
	HandleSourceControlStateChanged();
}

void SDirectoryTreeItem::HandleSourceControlStateChanged()
{
	if (this->TreeItem.Pin() && !this->TreeItem.Pin()->bIsDirectory && ISourceControlModule::Get().IsEnabled())
	{
		FString Path = this->TreeItem.Pin()->DiskPath;
		FSourceControlStatePtr SourceControlState = ISourceControlModule::Get().GetProvider().GetState(Path, EStateCacheUsage::Use);
		if (SourceControlState)
		{
			if (this->SCCStateWidget.IsValid())
			{
				this->SCCStateWidget->SetFromSlateIcon(SourceControlState->GetIcon());
			}
		}
	}
}
