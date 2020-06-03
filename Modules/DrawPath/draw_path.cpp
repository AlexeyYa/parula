
#include "Modules/DrawPath/draw_path.h"


Drawing::Drawing(ImageEngine* image_engine) :
    IModule(image_engine),
    m_stroke(nullptr)
{}

void Drawing::HandleEvent(std::shared_ptr<void*> data)
{
    cur_idx = 0;
    m_stroke = std::reinterpret_pointer_cast<IStroke>(data);
}

void Drawing::UpdateTextures()
{
    if (m_stroke == nullptr)
        return;

    auto layer = m_iengine->GetTemporaryLayer();
    m_pixels = layer.GetPixels();

    for (; cur_idx < m_stroke->stroke.size(); ++cur_idx)
    {

    }

    if (m_stroke->completed && cur_idx == m_stroke->stroke.size())
    {
        cur_idx = 0;
        m_stroke = nullptr;
    }

    m_pixels = nullptr;
    layer.FreePixels();
}
