
#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#include "Graphics/texture.h"
#include "Modules/imodule.h"
#include "Input/input_manager.h"

#include "Modules/Recognition/rec_worker.h"
#include "Modules/DrawPath/draw_path.h"

#include <SDL2/SDL.h>
#include <tbb/concurrent_vector.h>
#include <memory>

class ImageEngine
{
public:
    ImageEngine(int width, int height, SDL_Window* window, SDL_Renderer* renderer,
                std::shared_ptr<InputManager> input_manager);

    void SubscribeEvent(INPUTEVENT input_event, delegate function);
    void DrawFrame();
    void UpdateTextures();
    tbb::concurrent_vector<LTexture>& GetLayers();
    LTexture* GetTemporaryLayer();
private:
    int m_width;
    int m_height;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    std::shared_ptr<InputManager> m_input_manager;

    LTexture m_tmp_layer;

    tbb::concurrent_vector<LTexture> m_layers;

    tbb::concurrent_vector<std::unique_ptr<IModule>> m_modules;
};


#endif
