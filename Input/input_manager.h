#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "Input/iinput_state.h"
#include "Input/input_event.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include <boost/signals2.hpp>
#include <functional>
#include <memory>

class InputManager
{
public:
    InputManager();

    void Run();
    void SubscribeEvent(INPUTEVENT input_event, delegate function);
    void FireEvent(INPUTEVENT input_event, std::shared_ptr<void*> data) const;

    IInputState GetCurrentPointer();
private:
    void EventLoop();

    bool m_running;
    tbb::concurrent_unordered_map<INPUTEVENT, boost::signals2::signal<void(std::shared_ptr<void*>)>,
        std::hash<INPUTEVENT>> m_events;
};

#endif
