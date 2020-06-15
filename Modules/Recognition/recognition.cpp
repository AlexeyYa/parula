
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
            switch (sh->type)
            {
            case Shape::Type::Line:
                ptr = std::static_pointer_cast<Shape::Line>(sh);

                // Wrong color!!!
                dlib::draw_line((int)ptr->start.X, (int)ptr->start.Y, (int)ptr->end.X, (int)ptr->end.Y, layer, dlib::rgb_alpha_pixel{255, 255, 255, 255});
                layer->FreePixels();

                break;
            case Shape::Type::Ellipse:

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
