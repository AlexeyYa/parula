
#ifndef IINPUT_H
#define IINPUT_H

#include <tbb/concurrent_vector.h>
#include <atomic>
#include <condition_variable>

struct IInputState
{
    IInputState(float X, float Y, float TiltX, float TiltY, float NormalPressure) :
        X(X), Y(Y), TiltX(TiltX), TiltY(TiltY), NormalPressure(NormalPressure) {}

    float X;
    float Y;
    float TiltX;
    float TiltY;
    float NormalPressure;
};

enum class INPUT_DEVICE
{
    MOUSE,
    FINGER,
    STYLUS_NORMAL,
    STYLUS_INVERTED
};

struct IStroke
{
    IStroke(tbb::concurrent_vector<IInputState>&& stroke, INPUT_DEVICE source) :
        stroke(std::forward<tbb::concurrent_vector<IInputState>>(stroke)),
        source(source)
    {}

    tbb::concurrent_vector<IInputState> stroke;
    const INPUT_DEVICE source;
    std::atomic<bool> completed{false};
    std::condition_variable cv;
};

#endif
