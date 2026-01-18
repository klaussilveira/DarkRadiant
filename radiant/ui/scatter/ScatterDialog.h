#pragma once

#include "icommandsystem.h"
#include "iscatter.h"
#include "math/Vector3.h"
#include "wxutil/XmlResourceBasedWidget.h"
#include "wxutil/dialog/Dialog.h"

namespace ui {

class ScatterDialog : public wxutil::Dialog,
                      private wxutil::XmlResourceBasedWidget {
private:
  wxWindow *_densityLabel;
  wxWindow *_amountLabel;
  wxWindow *_minDistanceLabel;

public:
  ScatterDialog();

  ScatterDensityMethod getDensityMethod();
  ScatterDistribution getDistribution();

  float getDensity();
  int getAmount();

  float getMinDistance();
  int getSeed();

  ScatterFaceDirection getFaceDirection();

  float getRotationRange();
  bool getAlignToNormal();

  static void Show(const cmd::ArgumentList &args);

private:
  void onDensityMethodChanged(wxCommandEvent &ev);
  void onDistributionChanged(wxCommandEvent &ev);
  void updateControlVisibility();
};

} // namespace ui
