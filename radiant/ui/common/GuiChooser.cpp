#include "GuiChooser.h"

#include "i18n.h"
#include "ifilesystem.h"

#include "wxutil/Bitmap.h"

#include <wx/sizer.h>
#include <wx/button.h>

namespace ui
{

namespace
{
	const char* const WINDOW_TITLE = N_("Choose GUI Definition");

	const int WINDOW_WIDTH = 500;
	const int WINDOW_HEIGHT = 600;

	const char* const GUI_ICON = "sr_icon_readable.png";
	const char* const FOLDER_ICON = "folder16.png";

	const std::string GUI_DIR = "guis/";
	const std::string GUI_EXT = "gui";
}

GuiChooser::GuiChooser(const std::string& initialSelection) :
	DialogBase(_(WINDOW_TITLE), "GuiChooser"),
	_store(new wxutil::TreeModel(_columns)),
	_treeView(nullptr),
	_guiIcon(wxutil::GetLocalBitmap(GUI_ICON)),
	_folderIcon(wxutil::GetLocalBitmap(FOLDER_ICON)),
	_initialSelection(initialSelection)
{
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	populateWindow();
	fillTree();

	FindWindowById(wxID_OK, this)->Enable(false);
}

void GuiChooser::populateWindow()
{
	SetSizer(new wxBoxSizer(wxVERTICAL));

	wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
	GetSizer()->Add(vbox, 1, wxEXPAND | wxALL, 12);

	_treeView = wxutil::TreeView::CreateWithModel(this, _store.get(), wxDV_NO_HEADER);
	_treeView->AppendIconTextColumn(_("GUI Path"), _columns.name.getColumnIndex(),
		wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, wxALIGN_NOT, wxDATAVIEW_COL_SORTABLE);

	_treeView->AddSearchColumn(_columns.name);

	_treeView->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED,
		&GuiChooser::onSelectionChanged, this);
	_treeView->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED,
		&GuiChooser::onItemActivated, this);

	vbox->Add(_treeView, 1, wxEXPAND | wxBOTTOM, 6);
	vbox->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT);
}

void GuiChooser::fillTree()
{
	wxutil::VFSTreePopulator populator(_store);

	GlobalFileSystem().forEachFile(GUI_DIR, GUI_EXT,
		[&](const vfs::FileInfo& fileInfo)
		{
			populator.addPath(fileInfo.fullPath());
		},
		99
	);

	populator.forEachNode(*this);

	_store->SortModelFoldersFirst(_columns.name, _columns.isFolder);
}

void GuiChooser::visit(wxutil::TreeModel& /* store */, wxutil::TreeModel::Row& row,
	const std::string& path, bool isExplicit)
{
	std::string displayName = path.substr(path.rfind("/") + 1);

	if (isExplicit)
	{
		auto dotPos = displayName.rfind('.');
		if (dotPos != std::string::npos)
		{
			displayName = displayName.substr(0, dotPos);
		}
	}

	row[_columns.name] = wxVariant(wxDataViewIconText(displayName, isExplicit ? _guiIcon : _folderIcon));
	row[_columns.fullName] = path;
	row[_columns.isFolder] = !isExplicit;

	row.SendItemAdded();
}

void GuiChooser::selectInitialItem()
{
	if (_initialSelection.empty()) return;

	auto item = _store->FindString(_initialSelection, _columns.fullName);

	if (item.IsOk())
	{
		_treeView->Select(item);
		_treeView->EnsureVisible(item);
	}
}

int GuiChooser::ShowModal()
{
	selectInitialItem();
	return DialogBase::ShowModal();
}

void GuiChooser::onSelectionChanged(wxDataViewEvent&)
{
	auto item = _treeView->GetSelection();
	auto okButton = FindWindowById(wxID_OK, this);

	if (!item.IsOk())
	{
		_selection.clear();
		okButton->Enable(false);
		return;
	}

	wxutil::TreeModel::Row row(item, *_treeView->GetModel());

	if (row[_columns.isFolder].getBool())
	{
		_selection.clear();
		okButton->Enable(false);
		return;
	}

	_selection = row[_columns.fullName];
	okButton->Enable(true);
}

void GuiChooser::onItemActivated(wxDataViewEvent&)
{
	if (!_selection.empty())
	{
		EndModal(wxID_OK);
	}
}

std::string GuiChooser::ChooseGui(const std::string& prevSelection)
{
	auto* dialog = new GuiChooser(prevSelection);

	std::string result = prevSelection;

	if (dialog->ShowModal() == wxID_OK)
	{
		result = dialog->_selection;
	}

	dialog->Destroy();

	return result;
}

} // namespace
