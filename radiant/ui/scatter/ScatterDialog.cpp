#include "ScatterDialog.h"

#include "command/ExecutionNotPossible.h"
#include "i18n.h"
#include "selectionlib.h"
#include "string/convert.h"
#include "ui/imainframe.h"

#include <random>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace {
const char *const WINDOW_TITLE = N_("Scatter Objects");
}

namespace ui {

ScatterDialog::ScatterDialog()
    : Dialog(_(WINDOW_TITLE), GlobalMainFrame().getWxTopLevelWindow()),
      _densityLabel(nullptr), _amountLabel(nullptr),
      _minDistanceLabel(nullptr) {
  _dialog->GetSizer()->Add(loadNamedPanel(_dialog, "ScatterDialogMainPanel"), 1,
                           wxEXPAND | wxALL, 12);

  wxStaticText *topLabel =
      findNamedObject<wxStaticText>(_dialog, "ScatterDialogTopLabel");
  topLabel->SetFont(topLabel->GetFont().Bold());

  _densityLabel =
      findNamedObject<wxStaticText>(_dialog, "ScatterDialogDensityLabel");
  _amountLabel =
      findNamedObject<wxStaticText>(_dialog, "ScatterDialogAmountLabel");
  _minDistanceLabel =
      findNamedObject<wxStaticText>(_dialog, "ScatterDialogMinDistanceLabel");

  wxChoice *densityMethodChoice =
      findNamedObject<wxChoice>(_dialog, "ScatterDialogDensityMethod");
  densityMethodChoice->Bind(wxEVT_CHOICE,
                            &ScatterDialog::onDensityMethodChanged, this);

  wxChoice *distributionChoice =
      findNamedObject<wxChoice>(_dialog, "ScatterDialogDistribution");
  distributionChoice->Bind(wxEVT_CHOICE, &ScatterDialog::onDistributionChanged,
                           this);

  // Always start with a random seed
  std::random_device rd;
  findNamedObject<wxSpinCtrl>(_dialog, "ScatterDialogSeed")
      ->SetValue(rd() % 1000000);

  updateControlVisibility();
}

void ScatterDialog::onDensityMethodChanged(wxCommandEvent &ev) {
  updateControlVisibility();
  _dialog->Layout();
  _dialog->Fit();
}

void ScatterDialog::onDistributionChanged(wxCommandEvent &ev) {
  updateControlVisibility();
  _dialog->Layout();
  _dialog->Fit();
}

void ScatterDialog::updateControlVisibility() {
  ScatterDensityMethod method = getDensityMethod();
  ScatterDistribution dist = getDistribution();

  bool useDensity = (method == ScatterDensityMethod::Density);
  bool usePoisson = (dist == ScatterDistribution::PoissonDisk);

  // Show/hide density vs amount fields
  if (_densityLabel) {
    _densityLabel->Show(useDensity);
    findNamedObject<wxTextCtrl>(_dialog, "ScatterDialogDensity")
        ->Show(useDensity);
  }

  if (_amountLabel) {
    _amountLabel->Show(!useDensity);
    findNamedObject<wxSpinCtrl>(_dialog, "ScatterDialogAmount")
        ->Show(!useDensity);
  }

  // Show min distance only for Poisson Disk
  if (_minDistanceLabel) {
    _minDistanceLabel->Show(usePoisson);
    findNamedObject<wxTextCtrl>(_dialog, "ScatterDialogMinDistance")
        ->Show(usePoisson);
  }
}

ScatterDensityMethod ScatterDialog::getDensityMethod() {
  wxChoice *choice =
      findNamedObject<wxChoice>(_dialog, "ScatterDialogDensityMethod");
  return static_cast<ScatterDensityMethod>(choice->GetSelection());
}

ScatterDistribution ScatterDialog::getDistribution() {
  wxChoice *choice =
      findNamedObject<wxChoice>(_dialog, "ScatterDialogDistribution");
  return static_cast<ScatterDistribution>(choice->GetSelection());
}

float ScatterDialog::getDensity() {
  return string::convert<float>(
      findNamedObject<wxTextCtrl>(_dialog, "ScatterDialogDensity")
          ->GetValue()
          .ToStdString(),
      0.01f);
}

int ScatterDialog::getAmount() {
  return findNamedObject<wxSpinCtrl>(_dialog, "ScatterDialogAmount")
      ->GetValue();
}

float ScatterDialog::getMinDistance() {
  return string::convert<float>(
      findNamedObject<wxTextCtrl>(_dialog, "ScatterDialogMinDistance")
          ->GetValue()
          .ToStdString(),
      32.0f);
}

int ScatterDialog::getSeed() {
  return findNamedObject<wxSpinCtrl>(_dialog, "ScatterDialogSeed")->GetValue();
}

ScatterFaceDirection ScatterDialog::getFaceDirection() {
  wxChoice *choice =
      findNamedObject<wxChoice>(_dialog, "ScatterDialogFaceDirection");
  return static_cast<ScatterFaceDirection>(choice->GetSelection());
}

float ScatterDialog::getRotationRange() {
  return string::convert<float>(
      findNamedObject<wxTextCtrl>(_dialog, "ScatterDialogRotationRange")
          ->GetValue()
          .ToStdString(),
      360.0f);
}

bool ScatterDialog::getAlignToNormal() {
  return findNamedObject<wxCheckBox>(_dialog, "ScatterDialogAlignToNormal")
      ->GetValue();
}

void ScatterDialog::Show(const cmd::ArgumentList &args) {
  if (GlobalSelectionSystem().countSelected() == 0) {
    throw cmd::ExecutionNotPossible(
        _("Cannot scatter objects. Nothing selected."));
  }

  ScatterDialog dialog;

  if (dialog.run() == IDialog::RESULT_OK) {
    GlobalCommandSystem().executeCommand(
        "ScatterObjects",
        {cmd::Argument(static_cast<int>(dialog.getDensityMethod())),
         cmd::Argument(static_cast<int>(dialog.getDistribution())),
         cmd::Argument(dialog.getDensity()), cmd::Argument(dialog.getAmount()),
         cmd::Argument(dialog.getMinDistance()),
         cmd::Argument(dialog.getSeed()),
         cmd::Argument(static_cast<int>(dialog.getFaceDirection())),
         cmd::Argument(dialog.getRotationRange()),
         cmd::Argument(dialog.getAlignToNormal() ? 1 : 0)});
  }
}

} // namespace ui
