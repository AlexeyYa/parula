
#ifndef SHAPESTRUCT_H
#define SHAPESTRUCT_H

#include <memory>
#include <math.h>

namespace Shape
{
    struct Point
    {
        Point() : X(0), Y(0) {}
        Point(float X, float Y) : X(X), Y(Y) {}

        float X;
        float Y;
    };

    enum class Type
    {
        Line,
        Ellipse
    };

    struct Shape
    {
        Shape(Shape::Type type) : type(type) {}

        Shape::Type type;
    };

    struct Line : public Shape
    {
        Line(Point start, Point end) : Shape::Shape(Type::Line), start(start), end(end) {}
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
        Ellipse(double a, double b, double c, double d, double e, double f) : Shape::Shape(Type::Ellipse), a(a), b(b), c(c), d(d), e(e), f(f)
        {
            double tmp1 = b * b - 4 * a * c;
            double tmp2 = sqrt((a - c) * (a - c) + b * b);
            double tmp3 = a * e * e + c * d * d - b * d * e + tmp1 * f;
            double r1 = abs(sqrt(2 * tmp3 * (a + c + tmp2)) / tmp1);
            double r2 = abs(sqrt(2 * tmp3 * (a + c - tmp2)) / tmp1);

            rl = r1 >= r2 ? r1 : r2;
            rs = r1 <= r2 ? r1 : r2;

            center.X = (2 * c * d - b * e) / tmp1;
            center.Y = (2 * a * e - b * d) / tmp1;
            
            phi = 0.5 * atan2(b, a - c);
            if (r1 > r2)
                phi += 3.14159f / 2;
        }
        // f(x, y) = a*x^2 + b*x*y + c*y^2 + d*x + e*y + f
        double a, b, c, d, e, f;

        Point center;
        double rl;
        double rs;
        double phi;
    };

    //Container<std::shared_ptr<Shape>>
    template <typename Container>
    struct PolyShape
    {
        Container shapes;
    };
}
#endif
