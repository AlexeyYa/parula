
#ifndef SHAPESTRUCT_H
#define SHAPESTRUCT_H

#include <memory>

namespace Shape
{
    struct Point
    {
        Point(float X, float Y) : X(X), Y(Y) {}

        float X;
        float Y;
    };

    struct Shape
    {};

    struct Line : public Shape
    {
        Line(Point start, Point end) : start(start), end(end) {}
        Point start;
        Point end;
    };

    // Container<Point>
    template <typename Container>
    struct Bezier : public Shape
    {
        Container P;
    };

    struct Ellipse : public Shape
    {
        // f(x, y) = a*x^2 + b*x*y + c*y^2 + d*x + e*y + f
        float a, b, c, d, e, f;
        Point center;
        float r1;
        float r2;
        float phi;
    };

    //Container<std::shared_ptr<Shape>>
    template <typename Container>
    struct PolyShape
    {
        Container shapes;
    };
}
#endif
