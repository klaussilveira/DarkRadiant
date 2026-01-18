#pragma once

#include "icommandsystem.h"
#include "iarray.h"
#include "wxutil/dialog/Dialog.h"
#include "wxutil/XmlResourceBasedWidget.h"
#include "math/Vector3.h"

namespace ui
{

/**
 * Dialog for creating an array of copies of the selected objects.
 * Similar to Blender's Array modifier - creates multiple copies
 * with specified offset, rotation, and arrangement options.
 */
class ArrayDialog :
    public wxutil::Dialog,
    private wxutil::XmlResourceBasedWidget
{
private:
    wxWindow* _lineOffsetPanel;
    wxWindow* _circlePanel;
    wxWindow* _splinePanel;
    wxWindow* _rotationPanel;
    wxWindow* _offsetMethodLabel;
    wxWindow* _offsetMethodChoice;

public:
    ArrayDialog();

    // Get the number of copies to create
    int getCount();

    // Get the arrangement method
    ArrayArrangement getArrangement();

    // Get the offset method (for line arrangement)
    ArrayOffsetMethod getOffsetMethod();

    // Get the offset between each copy (for line arrangement)
    Vector3 getOffset();

    // Get the rotation (in degrees) to apply to each copy
    Vector3 getRotation();

    // Circle parameters
    float getRadius();
    float getStartAngle();
    float getEndAngle();
    bool getCircleRotate();

    // Spline parameters
    bool getSplineRotate();

    // Static method to show the dialog and execute the command
    static void Show(const cmd::ArgumentList& args);

private:
    void onArrangementChanged(wxCommandEvent& ev);
    void updatePanelVisibility();
    void findPanelsByName(wxWindow* window);
};

} // namespace ui
