#pragma once

namespace ui
{

enum class ArrayArrangement
{
    Line = 0,
    Circle = 1,
    Spline = 2
};

enum class ArrayOffsetMethod
{
    Fixed = 0,    // Fixed world-space distance
    Relative = 1, // Relative to bounding box size
    Endpoint = 2  // Distribute between start and end points
};

} // namespace
