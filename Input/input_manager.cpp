
#include "Input/input_manager.h"

// Stylus handler, Windows RealTimeStylus API
#ifdef _WIN32
#include "Input/StylusInput/stylus.h"
#endif

InputManager::InputManager()
{

}

void InputManager::Run()
{
    EventLoop();
}

void InputManager::SubscribeEvent(INPUTEVENT input_event, delegate &function)
{
    auto pair = std::make_pair(input_event, function);
    m_events[input_event].connect(function);
}

#include <iostream>
void InputManager::EventLoop()
{
    std::cout << "eventloop" << std::endl;
    SDL_Event event;
    SDL_EventState(SDL_SYSWMEVENT, 1);

    bool g_bTriedToCreateRTSHandler = false;
    while(m_running){
        //SDL_PumpEvents();

        while(SDL_PollEvent( &event )){
            switch( event.type )
            {
            case SDL_QUIT:
                m_running = false;
                break;

// Using RTS API for all inputs on windows
#ifndef _WIN32
            case SDL_FINGERDOWN:
                std::cout << "FingerDown" << std::endl;
                break;
            case SDL_FINGERUP:
                std::cout << "FingerUp" << std::endl;
                break;
            case SDL_MOUSEBUTTONDOWN:
                std::cout << "MouseDown" << std::endl;
                break;
            case SDL_MOUSEBUTTONUP:
                std::cout << "MouseUp" << std::endl;
                break;

            case SDL_FINGERMOTION:
                std::cout << "FingerMotion" << std::endl;
                break;
            case SDL_MOUSEMOTION:
                std::cout << "MouseMotion" << std::endl;
                break;
#endif
#ifdef _WIN32
            // Stylus handler creation event, Windows RealTimeStylus API
            case SDL_SYSWMEVENT:
                // We would normally do this in WM_CREATE but SDL intercepts that message so use WM_SETFOCUS instead
                SDL_SysWMmsg *pMsg = event.syswm.msg;
                if( pMsg )//&& (pMsg->msg.win.msg == WM_ACTIVATE) )
                {
                    if( !g_bTriedToCreateRTSHandler )
                    {
                        CreateRTS(pMsg->msg.win.hwnd, this);
                        SDL_EventState(SDL_SYSWMEVENT, 0);
                    }
                    g_bTriedToCreateRTSHandler = true;
                }
                break;
#endif
            }
        }
    }
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
