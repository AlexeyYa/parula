
#include "Modules/DrawPath/draw_path.h"

#include "Graphics/texture.h"

Drawing::Drawing(ImageEngine* image_engine) :
    IModule(image_engine),
    m_stroke(nullptr)
{
    m_iengine->SubscribeEvent(INPUTEVENT::STROKE_START, boost::bind(&Drawing::HandleEvent, this, _1));
}

void Drawing::HandleEvent(std::shared_ptr<void*> data)
{
    cur_idx = 1;
    m_stroke = std::reinterpret_pointer_cast<IStroke>(data);
}

void Drawing::UpdateTextures()
{
    if (m_stroke == nullptr)
        return;

    auto layer = m_iengine->GetTemporaryLayer();
    m_pixels = (int*)layer->GetPixels();
    m_pitch = layer->GetPitch();
    if (m_pixels == nullptr)
    {
        layer->FreePixels();
        return;
    }

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

    m_pixels = nullptr;
    layer->FreePixels();
}

void Drawing::DrawLine(Point start, Point end) const
{
    // calculate dx & dy
    float dx = end.X - start.X;
    float dy = end.Y - start.Y;
    if (dx < 1 && dy < 1)
    {
        m_pixels[(int)start.X + (int)start.Y * m_pitch / 4] = 255;
        m_pixels[(int)end.X + (int)end.Y * m_pitch / 4] = 255;
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
        m_pixels[(int)X + (int)Y * m_pitch / 4] = 255;  // put pixel at (X,Y)
        X += Xinc;           // increment in x at each step
        Y += Yinc;           // increment in y at each step
    }
}
