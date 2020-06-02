#include "window.h"

#include "Input/StylusInput/stylus.h"

#include <thread>
#include <chrono>


#include <iostream>
bool EditorWindow::Show()
{
    std::cout << "Renderer Created" << std::endl;
    m_running = true;

    std::cout << "PreRender" << std::endl;
    auto render_thread = std::thread(&EditorWindow::Render, this, 60.0f);
// Events!
    std::cout << "PreInput" << std::endl;
    m_input->Run();
    std::cout << "PostInput" << std::endl;

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
