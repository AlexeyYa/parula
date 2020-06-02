#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include "Input/input_manager.h"
#include "Graphics/image_engine.h"

#include <SDL2/SDL.h>

#include <atomic>
#include <memory>


#include <iostream>
class EditorWindow
{
public:
    EditorWindow(int width, int height, bool fullscreen) :
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

        m_input = std::make_shared<InputManager>();
        m_iengine = std::make_unique<ImageEngine>(m_width, m_height, m_window, m_renderer, m_input);

        std::cout << "Window created" << std::endl;
    }

    ~EditorWindow(){}

    bool Show();
    void Stop() { m_running = false; }
private:
    int m_width;
    int m_height;
    bool m_fullscreen;
    bool m_running; // -> atomic

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;

    void Update();
    void Render(float Hz);

    // Modules
    std::shared_ptr<InputManager> m_input;
    std::unique_ptr<ImageEngine> m_iengine;
};

#endif // WINDOW_H
