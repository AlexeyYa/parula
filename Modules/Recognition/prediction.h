
#ifndef PREDICTION_H
#define PREDICTION_H

#include "Modules/VectorGraphics/shapes.h"

#include <memory>
#include <tbb/concurrent_vector.h>
#include <vector>

struct RPrediction
{
    enum ShapeID{
        Unknown,
        Line,
        Bezier,
        Ellipse
    };

    ShapeID active = RPrediction::Unknown;

    std::shared_ptr<Shs::Shape> shape = nullptr;
    size_t start;
    size_t end;
};

#endif
