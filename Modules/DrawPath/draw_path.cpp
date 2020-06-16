
#include "Modules/DrawPath/draw_path.h"

#include "Graphics/texture.h"
#include <algorithm>

Drawing::Drawing(ImageEngine* image_engine, int width, int height) :
    IModule(image_engine),
    m_pixels(nullptr),
    m_pitch(0),
    m_width(width),
    m_height(height)
{
    m_iengine->SubscribeEvent(INPUTEVENT::STROKE_START, boost::bind(&Drawing::HandleEvent, this, boost::placeholders::_1));
}

void Drawing::HandleEvent(std::shared_ptr<void*> data)
{
    auto it = std::find_if(m_strokes.begin(), m_strokes.end(), [](const auto& elem) { return elem.second == 0; });
    if (it != m_strokes.end())
    {
        it->first = std::reinterpret_pointer_cast<IStroke>(data);
        it->second = 1;
    }
    else
    {
        m_strokes.emplace_back(std::reinterpret_pointer_cast<IStroke>(data), 1);
    }
}

void Drawing::UpdateTextures()
{
    if (m_strokes.empty())
        return;

    auto layer = m_iengine->GetTemporaryLayer();
    m_pixels = (int*)layer->GetPixels();
    m_pitch = layer->GetPitch();
    if (m_pixels == nullptr)
    {
        layer->FreePixels();
        return;
    }
    for (auto& s : m_strokes)
    {
        auto& [m_stroke, cur_idx] = s;
        if (m_stroke != nullptr)
        {
            for (; cur_idx < m_stroke->stroke.size(); ++cur_idx)
            {
                Point start(m_stroke->stroke[cur_idx - 1].X, m_stroke->stroke[cur_idx - 1].Y);
                Point end(m_stroke->stroke[cur_idx].X, m_stroke->stroke[cur_idx].Y);
                DrawLine(start, end);
            }


            if (m_stroke->completed && cur_idx == m_stroke->stroke.size())
            {
                cur_idx = 0;
                m_stroke = nullptr;
            }
        }
    }

    m_pixels = nullptr;
    layer->FreePixels();
}

// Temporary solution
void Drawing::DrawLine(Point start, Point end) const
{
    // calculate dx & dy
    float dx = end.X - start.X;
    float dy = end.Y - start.Y;
    if (dx < 1 && dy < 1)
    {
        if (start.X < m_width && start.Y < m_height)
            m_pixels[(int)start.X + (int)start.Y * m_pitch / 4] = 0x009900AA;
        if (end.X < m_width && end.Y < m_height)
            m_pixels[(int)end.X + (int)end.Y * m_pitch / 4] = 0x009900AA;
        return;
    }

    // calculate steps required for generating pixels
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    // calculate increment in x & y for each steps
    float Xinc = dx / (float) steps;
    float Yinc = dy / (float) steps;

    // Put pixel for each step
    float X = start.X;
    float Y = start.Y;
    for (int i = 0; i <= steps; i++)
    {
        if (X < m_width && Y < m_height)
            m_pixels[(int)X + (int)Y * m_pitch / 4] = 0x009900AA;  // put pixel at (X,Y)
        X += Xinc;           // increment in x at each step
        Y += Yinc;           // increment in y at each step
    }
}
