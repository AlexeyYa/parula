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
    EditorWindow(int width, int height, bool fullscreen);

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
