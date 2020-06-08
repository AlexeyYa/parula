
#ifndef POINT_H
#define POINT_H

struct Point
{
    Point(float X, float Y) :
        X(X), Y(Y)
    {}

    float X;
    float Y;
};

struct Rectangle
{
    Point p1;
    Point p2;
};

#endif
