
#ifndef PREDICTION_H
#define PREDICTION_H

#include "Modules/VectorGraphics/shapes.h"

#include <memory>
#include <tbb/concurrent_vector.h>
#include <vector>

struct RPrediction
{
    enum ShapeID{
        Line,
        Bezier,
        Ellipse,
        Unknown
    };

    ShapeID active = RPrediction::ShapeID::Unknown;

    std::shared_ptr<Shape> shape = nullptr;
    std::vector<float> angles;
};

#endif
