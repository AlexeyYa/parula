
#ifndef SHAPESTRUCT_H
#define SHAPESTRUCT_H

#include <memory>

struct Point
{
    Point(float X, float Y) : X(X), Y(Y) {}

    float X;
    float Y;
};

struct Shape
{

};

struct Line : public Shape
{
    Line(Point start, Point end) : start(start), end(end) {}
    Point start;
    Point end;
};
/*
template <typename Container>
struct Bezier : public Shape
{
    Container<Point> P;
};

struct Ellipse : public Shape
{
    // f(x, y) = a*x^2 + b*x*y + c*y^2 + d*x + e*y + f
    float a, b, c, d, e, f;
};

template <typename Contaner>
struct PolyShape
{
    Container<std::shared_ptr<Shape>> Shapes;
};
*/
#endif
