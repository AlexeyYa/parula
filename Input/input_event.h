
#ifndef INPUTEVENT_H
#define INPUTEVENT_H

#include <functional>
#include <memory>

typedef const std::function<void(std::shared_ptr<void*>)>& delegate;

enum class INPUTEVENT
{
    STROKE_START,
    TERMINATE
};

#endif
