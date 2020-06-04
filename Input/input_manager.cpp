
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
    m_running = true;
    EventLoop();
}

void InputManager::SubscribeEvent(INPUTEVENT input_event, delegate &function)
{
    auto pair = std::make_pair(input_event, function);
    m_events[input_event].connect(function);
}

void InputManager::EventLoop()
{
    SDL_Event event;
    SDL_EventState(SDL_SYSWMEVENT, 1);

#ifdef _WIN32
    bool g_bTriedToCreateRTSHandler = false;
#endif

    while(m_running){
        //SDL_PumpEvents();

        while(SDL_WaitEvent( &event )){
            switch( event.type )
            {
            case SDL_QUIT:
                m_running = false;
                break;

// Using RTS API for all inputs on windows
#ifndef _WIN32
            case SDL_FINGERDOWN:
                break;
            case SDL_FINGERUP:
                break;
            case SDL_MOUSEBUTTONDOWN:
                break;
            case SDL_MOUSEBUTTONUP:
                break;

            case SDL_FINGERMOTION:
                break;
            case SDL_MOUSEMOTION:
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
