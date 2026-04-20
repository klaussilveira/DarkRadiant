#include "CreateTrimDialog.h"

#include "i18n.h"
#include "icommandsystem.h"
#include "iselection.h"
#include "string/convert.h"
#include "selectionlib.h"
#include "wxutil/dialog/Dialog.h"
#include "wxutil/dialog/DialogElements.h"
#include "wxutil/dialog/MessageBox.h"

#include <wx/checkbox.h>
#include <wx/window.h>

namespace
{
	const char* WINDOW_TITLE = N_("Create Trim");
	const char* LABEL_HEIGHT = N_("Trim Height:");
	const char* LABEL_DEPTH = N_("Trim Depth:");
	const char* LABEL_FIT_TO = N_("Fit To:");
	const char* LABEL_MITERED = N_("45-degree mitered ends");
	const char* LABEL_COVER_ENTIRE_FACE = N_("Cover entire face");

	const double DEFAULT_HEIGHT = 16;
	const double DEFAULT_DEPTH = 1;

	const char* FIT_BOTTOM = N_("Bottom");
	const char* FIT_TOP = N_("Top");
	const char* FIT_LEFT = N_("Left");
	const char* FIT_RIGHT = N_("Right");

	// Values from the previous dialog invocation, restored on the next open
	double s_lastHeight = DEFAULT_HEIGHT;
	double s_lastDepth = DEFAULT_DEPTH;
	int s_lastFitToIndex = 0;
	bool s_lastMitered = false;
	bool s_lastCoverEntireFace = false;
}

namespace ui {

CreateTrimDialog::CreateTrimDialog() :
	wxutil::Dialog(_(WINDOW_TITLE))
{
	_heightHandle = addSpinButton(_(LABEL_HEIGHT), 1, 4096, 1, 0);
	_coverEntireFaceHandle = addCheckbox(_(LABEL_COVER_ENTIRE_FACE));
	_depthHandle = addSpinButton(_(LABEL_DEPTH), 1, 4096, 1, 0);

	ui::IDialog::ComboBoxOptions fitOptions;
	fitOptions.push_back(_(FIT_BOTTOM));
	fitOptions.push_back(_(FIT_TOP));
	fitOptions.push_back(_(FIT_LEFT));
	fitOptions.push_back(_(FIT_RIGHT));
	_fitToHandle = addComboBox(_(LABEL_FIT_TO), fitOptions);

	_miteredHandle = addCheckbox(_(LABEL_MITERED));

	setElementValue(_heightHandle, string::to_string(s_lastHeight));
	setElementValue(_depthHandle, string::to_string(s_lastDepth));

	const char* fitToStrs[] = { FIT_BOTTOM, FIT_TOP, FIT_LEFT, FIT_RIGHT };
	int idx = (s_lastFitToIndex >= 0 && s_lastFitToIndex < 4) ? s_lastFitToIndex : 0;
	setElementValue(_fitToHandle, _(fitToStrs[idx]));

	setElementValue(_coverEntireFaceHandle, s_lastCoverEntireFace ? "1" : "0");
	setElementValue(_miteredHandle, s_lastMitered ? "1" : "0");

	auto* coverCheckbox = dynamic_cast<wxCheckBox*>(_elements[_coverEntireFaceHandle]->getValueWidget());
	wxWindow* heightWidget = _elements[_heightHandle]->getValueWidget();

	if (coverCheckbox && heightWidget)
	{
		heightWidget->Enable(!coverCheckbox->GetValue());

		coverCheckbox->Bind(wxEVT_CHECKBOX, [heightWidget](wxCommandEvent& ev)
		{
			heightWidget->Enable(!ev.IsChecked());
			ev.Skip();
		});
	}
}

bool CreateTrimDialog::QueryTrimParams(TrimParams& params)
{
	CreateTrimDialog dialog;

	IDialog::Result result = dialog.run();

	if (result == IDialog::RESULT_OK)
	{
		params.height = string::convert<double>(dialog.getElementValue(dialog._heightHandle));
		params.depth = string::convert<double>(dialog.getElementValue(dialog._depthHandle));

		std::string fitToStr = dialog.getElementValue(dialog._fitToHandle);

		int fitToIndex = 0;

		if (fitToStr == _(FIT_TOP))
		{
			params.fitTo = FitTo::Top;
			fitToIndex = 1;
		}
		else if (fitToStr == _(FIT_LEFT))
		{
			params.fitTo = FitTo::Left;
			fitToIndex = 2;
		}
		else if (fitToStr == _(FIT_RIGHT))
		{
			params.fitTo = FitTo::Right;
			fitToIndex = 3;
		}
		else
		{
			params.fitTo = FitTo::Bottom;
			fitToIndex = 0;
		}

		params.mitered = (dialog.getElementValue(dialog._miteredHandle) == "1");
		params.coverEntireFace = (dialog.getElementValue(dialog._coverEntireFaceHandle) == "1");

		s_lastHeight = params.height;
		s_lastDepth = params.depth;
		s_lastFitToIndex = fitToIndex;
		s_lastMitered = params.mitered;
		s_lastCoverEntireFace = params.coverEntireFace;

		return true;
	}

	return false;
}

void CreateTrimDialog::CreateTrimCmd(const cmd::ArgumentList& args)
{
	if (GlobalSelectionSystem().getSelectionInfo().componentCount == 0)
	{
		wxutil::Messagebox::ShowError(_("Cannot create trim. No faces selected."));
		return;
	}

	TrimParams params;

	if (QueryTrimParams(params))
	{
		cmd::ArgumentList trimArgs;
		trimArgs.push_back(params.height);
		trimArgs.push_back(params.depth);
		trimArgs.push_back(static_cast<int>(params.fitTo));
		trimArgs.push_back(params.mitered ? 1 : 0);
		trimArgs.push_back(params.coverEntireFace ? 1 : 0);

		GlobalCommandSystem().executeCommand("CreateTrimForFaces", trimArgs);
	}
}

} // namespace
