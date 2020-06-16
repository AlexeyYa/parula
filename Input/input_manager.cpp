
#include "Input/input_manager.h"

// Stylus handler, Windows RealTimeStylus API
//#ifdef _WIN32
//#include "Input/StylusInput/stylus.h"
//#endif

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
                //INPUT_DEVICE device = INPUT_DEVICE::MOUSE;
                //event.
                //inputs[StylusInfo->cid] = std::make_shared<IStroke>(tbb::concurrent_vector<IInputState>(), device);
                break;
            case SDL_MOUSEBUTTONDOWN:

                break;
            case SDL_FINGERUP:
                //inputs[StylusInfo->cid]->completed.store(true);
                //inputs[StylusInfo->cid]->cv.notify_all();
                //inputs.erase(StylusInfo->cid);
                break;
            case SDL_MOUSEBUTTONUP:

                break;

            case SDL_FINGERMOTION:
                /*IInputState is(TabletContext->WindowsState.X,
                    TabletContext->WindowsState.Y,
                    TabletContext->WindowsState.NormalPressure,
                    TabletContext->WindowsState.TiltX,
                    TabletContext->WindowsState.TiltY);
                inputs.at(StylusInfo->cid)->stroke.push_back(is);
                inputs.at(StylusInfo->cid)->cv.notify_all();
                */break;
            case SDL_MOUSEMOTION:
                /*IInputState is(TabletContext->WindowsState.X,
                    TabletContext->WindowsState.Y,
                    TabletContext->WindowsState.NormalPressure,
                    TabletContext->WindowsState.TiltX,
                    TabletContext->WindowsState.TiltY);
                inputs.at(StylusInfo->cid)->stroke.push_back(is);
                inputs.at(StylusInfo->cid)->cv.notify_all();
                */break;
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
