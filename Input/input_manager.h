#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "Input/iinput_state.h"
#include "Input/input_event.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_unordered_map.h>
#include <boost/signals2.hpp>
#include <functional>
#include <memory>

class InputManager
{
public:
    InputManager(int width, int height);

    void Run();
    void SubscribeEvent(INPUTEVENT input_event, delegate function);
    void FireEvent(INPUTEVENT input_event, std::shared_ptr<void*> data) const;

    IInputState GetCurrentPointer();
private:
    void EventLoop();
    void PointerCoordEmplace(int x, int y, size_t id);

    std::map<size_t, std::shared_ptr<IStroke>> inputs;
    int m_width;
    int m_height;

    bool m_running = false;
    tbb::concurrent_unordered_map<INPUTEVENT, boost::signals2::signal<void(std::shared_ptr<void*>)>,
                    std::hash<INPUTEVENT>> m_events;
};

#endif
