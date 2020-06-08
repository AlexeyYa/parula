
#include "Modules/Recognition/recognition.h"
#include "Modules/Recognition/rec_worker.h"

#include <boost/signals2.hpp>
#include <mutex>

Recognition::Recognition(ImageEngine* image_engine, float threshold_angle, float threshold_distance) :
    IModule(image_engine),
    m_threshold_angle(threshold_angle),
    m_threshold_distance(threshold_distance)
{
    m_iengine->SubscribeEvent(INPUTEVENT::STROKE_START, boost::bind(&Recognition::HandleEvent, this, _1));
}

void Recognition::HandleEvent(std::shared_ptr<void *> data)
{
    std::make_shared<RecWorker>(this,std::reinterpret_pointer_cast<IStroke>(data),
                                m_threshold_distance, m_threshold_angle)->RecognizeStroke();

}

void Recognition::UpdateTextures()
{

}
