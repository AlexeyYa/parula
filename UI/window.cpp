#include "window.h"

#include "Input/StylusInput/stylus.h"

#include <thread>
#include <chrono>


#include <iostream>

EditorWindow::EditorWindow(int width, int height, bool fullscreen) :
    m_width(width),
    m_height(height),
    m_fullscreen(fullscreen)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){ throw; }

    if (m_fullscreen)
    {
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
            SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
            throw;
        }

        m_window = SDL_CreateWindow("Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              dm.w, dm.h, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    }
    else
    {
        m_window = SDL_CreateWindow("Editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              m_width, m_height, SDL_WINDOW_SHOWN);
    }
    if (m_window == nullptr) {
        SDL_Quit();
        throw;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, 0);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    m_input = std::make_shared<InputManager>(m_width, m_height);
    m_iengine = std::make_unique<ImageEngine>(m_width, m_height, m_window, m_renderer, m_input);

    std::cout << "Window created" << std::endl;
}

bool EditorWindow::Show()
{
    m_running = true;

    auto render_thread = std::thread(&EditorWindow::Render, this, 60.0f);
    m_input->Run();

    // Cleanup
    m_running = false;
    render_thread.join();

    SDL_DestroyWindow(m_window);
    SDL_Quit();

    return 0;
}

void EditorWindow::Update()
{
    m_iengine->UpdateTextures();
}

void EditorWindow::Render(float Hz)
{
    const int frameDelay = 1000 / Hz;

    Uint32 frameStart;
    int frameTime;

    while(m_running)
    {
        frameStart = SDL_GetTicks();

        Update();

        SDL_RenderClear(m_renderer);

        m_iengine->DrawFrame();
        SDL_RenderPresent(m_renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }
}
