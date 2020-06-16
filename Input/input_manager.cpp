
#include "Input/input_manager.h"
#include <iostream>
#include <algorithm>
#include <memory>

// Stylus handler, Windows RealTimeStylus API
//#ifdef _WIN32
//#include "Input/StylusInput/stylus.h"
//#endif

InputManager::InputManager(int width, int height) : m_width(width), m_height(height)
{

}

void InputManager::Run()
{
    m_running = true;
    EventLoop();
}

void InputManager::SubscribeEvent(INPUTEVENT input_event, delegate &function)
{
    auto pair = std::make_pair(input_event, function);
    m_events[input_event].connect(function);
}

void InputManager::PointerCoordEmplace(int x, int y, size_t id)
{
    inputs.at(id)->stroke.emplace_back(x, y, 1, 0, 0);
    inputs.at(id)->cv.notify_all();
}

void InputManager::EventLoop()
{
    SDL_Event event;
    INPUT_DEVICE device;
    bool clicked = false;
    int x, y;

//#ifdef _WIN32
//    SDL_EventState(SDL_SYSWMEVENT, 1);
//    bool g_bTriedToCreateRTSHandler = false;
//#endif

    while(m_running){
        //SDL_PumpEvents();

        while(SDL_WaitEventTimeout( &event, 200 )){
            switch( event.type )
            {
            case SDL_QUIT:
                m_running = false;
                FireEvent(INPUTEVENT::QUIT, nullptr);
                break;

// Using RTS API for all inputs on windows
//#ifndef _WIN32
            case SDL_FINGERDOWN:
                device = INPUT_DEVICE::FINGER;
                inputs[event.tfinger.fingerId] = std::make_shared<IStroke>(tbb::concurrent_vector<IInputState>(), device);

                x = event.tfinger.x * m_width;
                y = event.tfinger.y * m_height;
                PointerCoordEmplace(x, y, event.tfinger.fingerId);
                FireEvent(INPUTEVENT::STROKE_START, std::reinterpret_pointer_cast<void*>(inputs[event.tfinger.fingerId]));
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.which == 0)
                {
                    clicked = true;
                    device = INPUT_DEVICE::MOUSE;

                    inputs[0] = std::make_shared<IStroke>(tbb::concurrent_vector<IInputState>(), device);

                    x = event.button.x;
                    y = event.button.y;
                    
                    PointerCoordEmplace(x, y, 0);
                    FireEvent(INPUTEVENT::STROKE_START, std::reinterpret_pointer_cast<void*>(inputs[0]));
                }
                break;

            case SDL_FINGERUP:
                x = event.tfinger.x * m_width;
                y = event.tfinger.y * m_height;
                inputs[event.tfinger.fingerId]->completed.store(true);
                PointerCoordEmplace(x, y, event.tfinger.fingerId);
                inputs.erase(event.tfinger.fingerId);
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.which == 0)
                {
                    clicked = false;

                    x = event.button.x;
                    y = event.button.y;

                    inputs[0]->completed.store(true);
                    PointerCoordEmplace(x, y, 0);
                    inputs.erase(0);
                }
                break;

            case SDL_FINGERMOTION:
                x = event.tfinger.x * m_width;
                y = event.tfinger.y * m_height;

                PointerCoordEmplace(x, y, event.tfinger.fingerId);
                break;
            case SDL_MOUSEMOTION:
                if (event.button.which == 0)
                {
                    if (clicked)
                    {
                        x = event.button.x;
                        y = event.button.y;

                        PointerCoordEmplace(x, y, 0);
                    }
                }
                break;
//#endif
//#ifdef _WIN32
//            // Stylus handler creation event, Windows RealTimeStylus API
//            case SDL_SYSWMEVENT:
//                // We would normally do this in WM_CREATE but SDL intercepts that message so use WM_SETFOCUS instead
//                SDL_SysWMmsg *pMsg = event.syswm.msg;
//                if( pMsg )//&& (pMsg->msg.win.msg == WM_ACTIVATE) )
//                {
//                    if( !g_bTriedToCreateRTSHandler )
//                    {
//                        CreateRTS(pMsg->msg.win.hwnd, this);
//                        SDL_EventState(SDL_SYSWMEVENT, 0);
//                    }
//                    g_bTriedToCreateRTSHandler = true;
//                }
//                break;
//#endif
            }
        }
    }
//#ifdef _WIN32
//    ReleaseRTS();
//#endif
}

void InputManager::FireEvent(INPUTEVENT input_event, std::shared_ptr<void*> data) const
{
    auto range =m_events.equal_range(input_event);
    for (auto it = range.first; it != range.second; it++)
    {
        it->second(data);
    }
}

IInputState InputManager::GetCurrentPointer()
{
    throw;
}
