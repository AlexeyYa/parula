
#include "Graphics/image_engine.h"
#include "Modules/Recognition/recognition.h"
#include "Modules/DrawPath/draw_path.h"

ImageEngine::ImageEngine(int width, int height, SDL_Window* window, SDL_Renderer* renderer,
            std::shared_ptr<InputManager> input_manager) :
    m_width(width),
    m_height(height),
    m_window(window),
    m_renderer(renderer),
    m_input_manager(input_manager),
    m_tmp_layer(m_width, m_height, m_renderer)
{
    m_modules.push_back(std::make_unique<Recognition>(this, 10.0f, 3.1415f/16));
    m_modules.push_back(std::make_unique<Drawing>(this));

    m_layers.emplace_back(m_width, m_height, m_renderer);
    m_layers.front().Fill(255, 255, 255, 255);
}

void ImageEngine::SubscribeEvent(INPUTEVENT input_event, delegate function)
{
    m_input_manager->SubscribeEvent(input_event, function);
}

void ImageEngine::DrawFrame()
{
    for (const auto& layer : m_layers)
    {
        layer.Render();
    }

    m_tmp_layer.Render();
}

void ImageEngine::UpdateTextures()
{
    for (auto& module : m_modules)
    {
        module->UpdateTextures();
    }
}

tbb::concurrent_vector<LTexture>& ImageEngine::GetLayers()
{
    return m_layers;
}

LTexture* ImageEngine::GetTemporaryLayer()
{
    return &m_tmp_layer;
}
