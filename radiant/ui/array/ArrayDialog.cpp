#include "ArrayDialog.h"

#include "i18n.h"
#include "ui/imainframe.h"
#include "string/convert.h"
#include "selectionlib.h"
#include "command/ExecutionNotPossible.h"

#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>

namespace
{
    const char* const WINDOW_TITLE = N_("Create Array");
}

namespace ui
{

ArrayDialog::ArrayDialog() :
    Dialog(_(WINDOW_TITLE), GlobalMainFrame().getWxTopLevelWindow()),
    _lineOffsetPanel(nullptr),
    _circlePanel(nullptr),
    _splinePanel(nullptr),
    _rotationPanel(nullptr),
    _offsetMethodLabel(nullptr),
    _offsetMethodChoice(nullptr)
{
    _dialog->GetSizer()->Add(loadNamedPanel(_dialog, "ArrayDialogMainPanel"), 1, wxEXPAND | wxALL, 12);

    wxStaticText* topLabel = findNamedObject<wxStaticText>(_dialog, "ArrayDialogTopLabel");
    topLabel->SetFont(topLabel->GetFont().Bold());

    // Get panel references for visibility control
    // The static box sizers are the grandparents of our content - find them via their label
    for (wxWindowList::iterator it = _dialog->GetChildren().begin(); it != _dialog->GetChildren().end(); ++it)
    {
        wxWindow* child = *it;
        findPanelsByName(child);
    }

    // Get the offset method controls
    _offsetMethodLabel = findNamedObject<wxStaticText>(_dialog, "ArrayDialogOffsetMethodLabel");
    _offsetMethodChoice = findNamedObject<wxChoice>(_dialog, "ArrayDialogOffsetMethod");

    // Bind arrangement change event
    wxChoice* arrangementChoice = findNamedObject<wxChoice>(_dialog, "ArrayDialogArrangement");
    arrangementChoice->Bind(wxEVT_CHOICE, &ArrayDialog::onArrangementChanged, this);

    // Initial visibility update
    updatePanelVisibility();
}

void ArrayDialog::findPanelsByName(wxWindow* window)
{
    if (!window) return;

    wxString name = window->GetName();
    if (name == "ArrayDialogLineOffsetSizer")
        _lineOffsetPanel = window;
    else if (name == "ArrayDialogCircleSizer")
        _circlePanel = window;
    else if (name == "ArrayDialogSplineSizer")
        _splinePanel = window;
    else if (name == "ArrayDialogRotationSizer")
        _rotationPanel = window;

    // Recurse into children
    for (wxWindowList::iterator it = window->GetChildren().begin(); it != window->GetChildren().end(); ++it)
    {
        findPanelsByName(*it);
    }
}

void ArrayDialog::onArrangementChanged(wxCommandEvent& ev)
{
    updatePanelVisibility();
    _dialog->Layout();
    _dialog->Fit();
}

void ArrayDialog::updatePanelVisibility()
{
    ArrayArrangement arrangement = getArrangement();

    bool isLine = (arrangement == ArrayArrangement::Line);
    bool isCircle = (arrangement == ArrayArrangement::Circle);
    bool isSpline = (arrangement == ArrayArrangement::Spline);

    // Show/hide offset method dropdown (only for line mode)
    if (_offsetMethodLabel) _offsetMethodLabel->Show(isLine);
    if (_offsetMethodChoice) _offsetMethodChoice->Show(isLine);

    // Show/hide panels based on arrangement
    if (_lineOffsetPanel) _lineOffsetPanel->Show(isLine);
    if (_circlePanel) _circlePanel->Show(isCircle);
    if (_splinePanel) _splinePanel->Show(isSpline);

    // Rotation panel is shown for Line mode only (Circle and Spline have their own rotation options)
    if (_rotationPanel) _rotationPanel->Show(isLine);
}

int ArrayDialog::getCount()
{
    wxSpinCtrl* spinner = findNamedObject<wxSpinCtrl>(_dialog, "ArrayDialogCount");
    return spinner->GetValue();
}

ArrayArrangement ArrayDialog::getArrangement()
{
    wxChoice* choice = findNamedObject<wxChoice>(_dialog, "ArrayDialogArrangement");
    return static_cast<ArrayArrangement>(choice->GetSelection());
}

ArrayOffsetMethod ArrayDialog::getOffsetMethod()
{
    wxChoice* choice = findNamedObject<wxChoice>(_dialog, "ArrayDialogOffsetMethod");
    return static_cast<ArrayOffsetMethod>(choice->GetSelection());
}

Vector3 ArrayDialog::getOffset()
{
    float x = string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogOffsetX")->GetValue().ToStdString(), 0.0f);
    float y = string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogOffsetY")->GetValue().ToStdString(), 0.0f);
    float z = string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogOffsetZ")->GetValue().ToStdString(), 0.0f);

    return Vector3(x, y, z);
}

Vector3 ArrayDialog::getRotation()
{
    float x = string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogRotateX")->GetValue().ToStdString(), 0.0f);
    float y = string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogRotateY")->GetValue().ToStdString(), 0.0f);
    float z = string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogRotateZ")->GetValue().ToStdString(), 0.0f);

    return Vector3(x, y, z);
}

float ArrayDialog::getRadius()
{
    return string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogRadius")->GetValue().ToStdString(), 128.0f);
}

float ArrayDialog::getStartAngle()
{
    return string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogStartAngle")->GetValue().ToStdString(), 0.0f);
}

float ArrayDialog::getEndAngle()
{
    return string::convert<float>(
        findNamedObject<wxTextCtrl>(_dialog, "ArrayDialogEndAngle")->GetValue().ToStdString(), 360.0f);
}

bool ArrayDialog::getCircleRotate()
{
    return findNamedObject<wxCheckBox>(_dialog, "ArrayDialogCircleRotate")->GetValue();
}

bool ArrayDialog::getSplineRotate()
{
    return findNamedObject<wxCheckBox>(_dialog, "ArrayDialogSplineRotate")->GetValue();
}

void ArrayDialog::Show(const cmd::ArgumentList& args)
{
    if (GlobalSelectionSystem().countSelected() == 0)
    {
        throw cmd::ExecutionNotPossible(_("Cannot create array. Nothing selected."));
    }

    ArrayDialog dialog;

    if (dialog.run() == IDialog::RESULT_OK)
    {
        ArrayArrangement arrangement = dialog.getArrangement();
        int count = dialog.getCount();

        switch (arrangement)
        {
        case ArrayArrangement::Line:
            GlobalCommandSystem().executeCommand("ArrayCloneSelectionLine",
                { cmd::Argument(count),
                  cmd::Argument(static_cast<int>(dialog.getOffsetMethod())),
                  cmd::Argument(dialog.getOffset()),
                  cmd::Argument(dialog.getRotation()) }
            );
            break;

        case ArrayArrangement::Circle:
            GlobalCommandSystem().executeCommand("ArrayCloneSelectionCircle",
                { cmd::Argument(count),
                  cmd::Argument(dialog.getRadius()),
                  cmd::Argument(dialog.getStartAngle()),
                  cmd::Argument(dialog.getEndAngle()),
                  cmd::Argument(dialog.getCircleRotate() ? 1 : 0) }
            );
            break;

        case ArrayArrangement::Spline:
            GlobalCommandSystem().executeCommand("ArrayCloneSelectionSpline",
                { cmd::Argument(count),
                  cmd::Argument(dialog.getSplineRotate() ? 1 : 0) }
            );
            break;
        }
    }
}

} // namespace ui
