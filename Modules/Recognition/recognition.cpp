
#include "Modules/Recognition/recognition.h"
#include "Modules/Recognition/rec_worker.h"

#include <boost/signals2.hpp>
#include <mutex>


// Temporary drawing there
#include <dlib/image_processing.h>

long num_rows(LTexture* texture) { return texture->m_height; }
long num_columns(LTexture* texture) { return texture->m_width; }
void set_image_size(LTexture* texture) {}
void* image_data(LTexture* texture)
{
    return texture->GetPixels();
}
//const void* image_data(LTexture texture){ return texture.GetPixels(); }
long width_step(LTexture* texture){ return texture->GetPitch(); }
//void swap(LTexture t1, LTexture t2){}

namespace dlib
{
    template <>
    struct image_traits<LTexture*>
    {
        typedef rgb_alpha_pixel pixel_type;
    };
}


Recognition::Recognition(ImageEngine* image_engine, float threshold_distance, float threshold_angle) :
    IModule(image_engine),
    m_threshold_distance(threshold_distance),
    m_threshold_angle(threshold_angle)
{
    m_iengine->SubscribeEvent(INPUTEVENT::STROKE_START, boost::bind(&Recognition::HandleEvent, this, boost::placeholders::_1));
}

void Recognition::HandleEvent(std::shared_ptr<void *> data)
{
    std::make_shared<RecWorker>(this,std::reinterpret_pointer_cast<IStroke>(data),
                                m_threshold_distance, m_threshold_angle)->RecognizeStroke();

}

void Recognition::UpdateTextures()
{
    auto layer = &m_iengine->GetLayers()[0];
    /*auto m_pixels = (int*)layer->GetPixels();
    auto m_pitch = layer->GetPitch();
    if (m_pixels == nullptr)
    {
        layer->FreePixels();
        return;
    }*/
    while (!updateQueue.empty())
    {
        std::shared_ptr<Shape::Shape> sh;
        if (updateQueue.try_pop(sh))
        {
            std::shared_ptr<Shape::Line> ptr;
            std::shared_ptr<Shape::Ellipse> ptrE;
            double x0;
            double y0;
            double x1;
            double y1;
            auto xp = [](std::shared_ptr<Shape::Ellipse> ell, float a) { return ell->rl * cos(a) * cos(ell->phi) - ell->rs * sin(a) * sin(ell->phi) + ell->center.X; };
            auto yp = [](std::shared_ptr<Shape::Ellipse> ell, float a) { return ell->rl * cos(a) * sin(ell->phi) + ell->rs * sin(a) * cos(ell->phi) + ell->center.Y; };
            switch (sh->T)
            {
            case Shape::Type::Line:
                ptr = std::static_pointer_cast<Shape::Line>(sh);

                // Wrong color!!!
                dlib::draw_line((int)ptr->start.X, (int)ptr->start.Y, (int)ptr->end.X, (int)ptr->end.Y, layer, dlib::rgb_alpha_pixel{255, 255, 255, 255});
                layer->FreePixels();

                break;
            case Shape::Type::Ellipse:
                ptrE = std::static_pointer_cast<Shape::Ellipse>(sh);

                // Parametric form

                x0 = xp(ptrE, 0);
                y0 = yp(ptrE, 0);
                // Drawing lines
                for (float a = 0.2; a < 6.35; a += 0.2)
                {
                    x1 = xp(ptrE, a);
                    y1 = yp(ptrE, a);

                    dlib::draw_line((int)x0, (int)y0, (int)x1, (int)y1, layer, dlib::rgb_alpha_pixel{ 255, 255, 255, 255 });
                    layer->FreePixels();

                    x0 = x1;
                    y0 = y1;
                }

                break;
            default:
                break;
            }
        }
        else
        {
            break;
        }
    }

    //layer->FreePixels();
}


void Recognition::AddShape(std::shared_ptr<Shape::Shape> sh)
{
    updateQueue.emplace(sh);
}
